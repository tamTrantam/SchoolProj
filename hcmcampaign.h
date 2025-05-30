/*
 * Ho Chi Minh City University of Technology
 * Faculty of Computer Science and Engineering
 * Initial code for Assignment 2
 * Programming Fundamentals Spring 2025
 * Date: 02.02.2025
 */

#ifndef _H_HCM_CAMPAIGN_H_
#define _H_HCM_CAMPAIGN_H_
#include "main.h"

using namespace std;

// Helper functions
static inline bool isPerfectSquare(int n) {
    if (n < 0) return false;
    int r = static_cast<int>(sqrt(n) + 0.5);
    return r * r == n;
}
static inline int digitSum(int n) { int s = 0; while (n) { s += n % 10; n /= 10; } return s; }
static inline int personalNumber(int score, int year = 1975) {
    int t = score;
    while (year) { t += year % 10; year /= 10; }
    while (t >= 10) t = digitSum(t);
    return t;
}
static bool isSpecialNumber(int S) {
    for (int base : {3, 5, 7}) {
        int n = S; bool ok = true;
        while (n) { if (n % base > 1) { ok = false; break; } n /= base; }
        if (ok) return true;
    }
    return false;
}

enum VehicleType { TRUCK, MORTAR, ANTIAIRCRAFT, ARMOREDCAR, APC, ARTILLERY, TANK };
enum InfantryType { SNIPER, ANTIAIRCRAFTSQUAD, MORTARSQUAD, ENGINEER, SPECIALFORCES, REGULARINFANTRY };

class Position {
    int r_, c_;
public:
    Position(int r = 0, int c = 0) : r_(r), c_(c) {}
    Position(const string& s) { sscanf(s.c_str(), "(%d,%d)", &r_, &c_); }
    int getRow() const { return r_; }
    int getCol() const { return c_; }
    void setRow(int r) { r_ = r; }
    void setCol(int c) { c_ = c; }
    string str() const { return "(" + to_string(r_) + "," + to_string(c_) + ")"; }
    double dist(const Position& p) const { int dr = r_ - p.r_, dc = c_ - p.c_; return sqrt(double(dr * dr + dc * dc)); }
};

class Unit {
    friend class UnitList;
protected:
    int quantity, weight;
    Position pos;
public:
    Unit(int quantity, int weight, Position pos) : quantity(quantity), weight(weight), pos(pos) {}
    virtual ~Unit() {}
    virtual int getAttackScore() const = 0;
    virtual bool isVehicle() const = 0;
    Position getCurrentPosition() const { return pos; }
    virtual string str() const = 0;
    int getQuantity() const { return quantity; }
    void scaleQuantity(double factor) { quantity = (int)ceil(quantity * factor); if (quantity < 0) quantity = 0; }
    void scaleWeight(double factor) { weight = (int)ceil(weight * factor); if (weight < 0) weight = 0; }
    void setWeight(int w) { weight = w; }
};

class Vehicle : public Unit {
    VehicleType type;
public:
    Vehicle(int q, int w, const Position& p, VehicleType t) : Unit(q, w, p), type(t) {}
    VehicleType getType() const { return type; }
    bool isVehicle() const override { return true; }
    int getAttackScore() const override {
        return (int)ceil((type * 304 + 1.0 * quantity * weight) / 30.0);
    }
    string str() const override {
        static const char* name[] = { "TRUCK","MORTAR","ANTIAIRCRAFT","ARMOREDCAR","APC","ARTILLERY","TANK" };
        ostringstream oss;
        oss << "Vehicle[vehicleType=" << name[type]
            << ",quantity=" << quantity
            << ",weight=" << weight
            << ",position=" << pos.str() << "]";
        return oss.str();
    }
};

class Infantry : public Unit {
    InfantryType type;
public:
    Infantry(int q, int w, const Position& p, InfantryType t) : Unit(q, w, p), type(t) { applyRule(); }
    InfantryType getType() const { return type; }
    bool isVehicle() const override { return false; }
    int getAttackScore() const override {
        int score = type * 56 + quantity * weight;
        if (type == SPECIALFORCES && isPerfectSquare(weight)) score += 75;
        return score;
    }
    string str() const override {
        static const char* name[] = { "SNIPER","ANTIAIRCRAFTSQUAD","MORTARSQUAD","ENGINEER","SPECIALFORCES","REGULARINFANTRY" };
        ostringstream oss;
        oss << "Infantry[infantryType=" << name[type]
            << ",quantity=" << quantity
            << ",weight=" << weight
            << ",position=" << pos.str() << "]";
        return oss.str();
    }
private:
    void applyRule() {
        int pnum = personalNumber(getAttackScore());
        if (pnum > 7)      quantity += (int)ceil(quantity * 0.2);
        else if (pnum < 3) quantity -= (int)ceil(quantity * 0.1);
        if (quantity < 0) quantity = 0;
    }
};

class UnitList {
    struct Node {
        Unit* u;
        Node* next;
        explicit Node(Unit* _u) : u(_u), next(nullptr) {}
    };
    Node* head = nullptr;
    Node* tail = nullptr;
    int   vCnt = 0, iCnt = 0;
    int   cap;
public:
    explicit UnitList(int c) : cap(c) {}
    ~UnitList() { clear(); }
    void clear() {
        while (head) {
            Node* n = head->next;
            delete head;
            head = n;
        }
        tail = nullptr;
        vCnt = iCnt = 0;
    }
    bool insert(Unit* u) {
        if (merge(u)) return true;
        if (vCnt + iCnt >= cap) return false;
        Node* n = new Node(u);
        if (u->isVehicle()) {
            if (!tail) head = tail = n;
            else { tail->next = n; tail = n; }
            ++vCnt;
        }
        else {
            n->next = head;
            head = n;
            if (!tail) tail = n;
            ++iCnt;
        }
        return true;
    }
    bool merge(Unit* u) {
        for (Node* p = head; p; p = p->next) {
            if (u->isVehicle() != p->u->isVehicle()) continue;
            if (u->isVehicle()) {
                auto newV = static_cast<Vehicle*>(u);
                auto oldV = static_cast<Vehicle*>(p->u);
                if (newV->getType() == oldV->getType()) {
                    double f = 1.0 + double(newV->getQuantity()) / oldV->getQuantity();
                    oldV->scaleQuantity(f);
                    return true;
                }
            }
            else {
                auto newI = static_cast<Infantry*>(u);
                auto oldI = static_cast<Infantry*>(p->u);
                if (newI->getType() == oldI->getType()) {
                    oldI->quantity += newI->getQuantity();
                    int sc = oldI->getAttackScore();
                    int pnum = personalNumber(sc);
                    if (pnum > 7)      oldI->quantity = (int)ceil(oldI->quantity * 1.2);
                    else if (pnum < 3) oldI->quantity = (int)ceil(oldI->quantity * 0.9);
                    return true;
                }
            }
        }
        return false;
    }
    template<typename F>
    void forEach(F f) const { for (Node* p = head; p; p = p->next) f(p->u); }
    template<typename P>
    vector<Unit*> subset(P pred) const {
        vector<Unit*> v;
        forEach([&](Unit* u) { if (pred(u)) v.push_back(u); });
        return v;
    }
    void remove(const std::vector<Unit*>& vec) {
        for (Unit* target : vec) {
            Node* prev = nullptr;
            Node* curr = head;
            while (curr && curr->u != target) {
                prev = curr;
                curr = curr->next;
            }
            if (!curr) continue;
            if (prev) prev->next = curr->next; else head = curr->next;
            if (curr == tail) tail = prev;
            (curr->u->isVehicle() ? --vCnt : --iCnt);
            delete curr;
        }
    }
    vector<Unit*> extractAll() {
        vector<Unit*> out;
        for (Node* p = head; p; p = p->next) out.push_back(p->u);
        clear();
        return out;
    }
    int vehicles()   const { return vCnt; }
    int infantries() const { return iCnt; }
    string str() const {
        ostringstream oss;
        oss << "UnitList[count_vehicle=" << vCnt
            << ";count_infantry=" << iCnt;
        if (head) {
            oss << ";";
            bool first = true;
            forEach([&](Unit* u) {
                if (!first) oss << ",";
                first = false;
                oss << u->str();
                });
        }
        oss << "]";
        return oss.str();
    }
};

class Army {
protected:
    int LF = 0, EXP = 0;
    string name;
    UnitList* list;
public:
    Army(const string& n) : name(n), list(0) {}
    virtual ~Army() { if (list) delete list; }
    int getLF()  const { return LF; }
    int getEXP() const { return EXP; }
    void setLF(int x) { LF = x < 0 ? 0 : x; if (LF > 1000) LF = 1000; }
    void setEXP(int x) { EXP = x < 0 ? 0 : x; if (EXP > 500) EXP = 500; }
    UnitList* units() const { return list; }
    void update() {
        int lf = 0, ex = 0;
        list->forEach([&](Unit* u) { (u->isVehicle() ? lf : ex) += u->getAttackScore(); });
        setLF(lf); setEXP(ex);
    }
    virtual void fight(Army*, bool) = 0;
    virtual bool isLiberation() const = 0;
    virtual string str() const = 0;
};

class TerrainElement {
protected: Position pos;
public: TerrainElement(const Position& p) : pos(p) {} virtual ~TerrainElement() {} virtual void getEffect(Army*) = 0;
};

class Mountain : public TerrainElement { public: Mountain(const Position& p) : TerrainElement(p) {} void getEffect(Army*) override; };
class River : public TerrainElement { public: River(const Position& p) : TerrainElement(p) {} void getEffect(Army*) override; };
class Urban : public TerrainElement { public: Urban(const Position& p) : TerrainElement(p) {} void getEffect(Army*) override; };
class Fortification : public TerrainElement { public: Fortification(const Position& p) : TerrainElement(p) {} void getEffect(Army*) override; };
class SpecialZone : public TerrainElement { public: SpecialZone(const Position& p) : TerrainElement(p) {} void getEffect(Army*) override; };
class Road : public TerrainElement { public: Road(const Position& p) : TerrainElement(p) {} void getEffect(Army*) override {} };

class BattleField {
    int R, C; vector<TerrainElement*> elems;
public:
    BattleField(int r, int c,
        const vector<Position*>& f, const vector<Position*>& rv,
        const vector<Position*>& fo, const vector<Position*>& ub,
        const vector<Position*>& sp) : R(r), C(c) {
        for (auto p : f) elems.push_back(new Mountain(*p));
        for (auto p : rv) elems.push_back(new River(*p));
        for (auto p : fo) elems.push_back(new Fortification(*p));
        for (auto p : ub) elems.push_back(new Urban(*p));
        for (auto p : sp) elems.push_back(new SpecialZone(*p));
    }
    ~BattleField() { for (size_t i = 0; i < elems.size(); ++i) delete elems[i]; }
    void apply(Army* a) { for (size_t i = 0; i < elems.size(); ++i) elems[i]->getEffect(a); }
    string str() const { return "BattleField[n_rows=" + to_string(R) + ",n_cols=" + to_string(C) + "]"; }
};

class LiberationArmy : public Army {
    BattleField* bf;
public:
    LiberationArmy(Unit** arr, int sz, BattleField* b) : Army("LIBERATIONARMY"), bf(b) { build(arr, sz); }
    LiberationArmy(Unit** arr, int sz, const string&, BattleField* b = 0) : Army("LIBERATIONARMY"), bf(b) { build(arr, sz); }
    bool isLiberation() const override { return true; }
    void fight(Army* enemy, bool defense = false) override;
    string str() const override {
        ostringstream oss;
        oss << "LiberationArmy[LF=" << LF
            << ",EXP=" << EXP
            << ",unitList=" << list->str()
            << ",battleField=]";
        return oss.str();
    }
private:
    void build(Unit** arr, int sz);
    static pair<int, vector<Unit*> > bestCombo(const vector<Unit*>& v, int need);
};

class ARVN : public Army {
    BattleField* bf;
public:
    ARVN(Unit** arr, int sz, BattleField* b) : Army("ARVN"), bf(b) { build(arr, sz); }
    ARVN(Unit** arr, int sz, const string&, BattleField* b = 0) : Army("ARVN"), bf(b) { build(arr, sz); }
    bool isLiberation() const override { return false; }
    void fight(Army* enemy, bool defense = false) override;
    string str() const override {
        ostringstream oss;
        oss << "ARVN[LF=" << LF
            << ",EXP=" << EXP
            << ",unitList=" << list->str()
            << ",battleField=]";
        return oss.str();
    }
private:
    void build(Unit** arr, int sz);
};

class Configuration {
    int num_rows = 0, num_cols = 0;
    vector<Position*> arrayForest, arrayRiver, arrayFortification, arrayUrban, arraySpecialZone;
    vector<Unit*> liberationUnits, ARVNUnits;
    int eventCode = 0;
public:
    Configuration(const string& path) { parseFile(path); }
    ~Configuration();
    int getNumRows()  const { return num_rows; }
    int getNumCols()  const { return num_cols; }
    const vector<Position*>& getForestPositions()        const { return arrayForest; }
    const vector<Position*>& getRiverPositions()         const { return arrayRiver; }
    const vector<Position*>& getFortificationPositions() const { return arrayFortification; }
    const vector<Position*>& getUrbanPositions()         const { return arrayUrban; }
    const vector<Position*>& getSpecialZonePositions()   const { return arraySpecialZone; }
    const vector<Unit*>& getLiberationUnits() const { return liberationUnits; }
    const vector<Unit*>& getARVNUnits()       const { return ARVNUnits; }
    int getEventCode() const { return eventCode; }
    vector<Unit*> stealLiberation() { vector<Unit*> tmp; tmp.swap(liberationUnits); return tmp; }
    vector<Unit*> stealARVN() { vector<Unit*> tmp; tmp.swap(ARVNUnits); return tmp; }
    string str() const;
private:
    static std::string trim(std::string s);
    static string vecPosStr(const vector<Position*>& v);
    static string vecUnitStr(const vector<Unit*>& v);
    void parsePosArray(const string& raw, vector<Position*>& dst);
    Unit* makeUnit(const string& rawToken);
    void parseFile(const string& path);
};

class HCMCampaign {
    Configuration* cfg; BattleField* bf;
    LiberationArmy* lib; ARVN* arvn;
public:
    explicit HCMCampaign(const string& path);
    ~HCMCampaign();
    void run();
    string printResult() const;
};

#endif // _H_HCM_CAMPAIGN_H_
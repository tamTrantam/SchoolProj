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

////////////////////////////////////////////////////////////////////////
/// STUDENT'S ANSWER BEGINS HERE
////////////////////////////////////////////////////////////////////////

// Forward declaration
class Position;
class Unit;
class Vehicle;
class Infantry;
class TerrainElement;
class BattleField;
class UnitList;
class Army;
class LiberationArmy;
class ARVN;

constexpr double vp = 0.10; // Vehicle penalty multiplier for Mountain
constexpr double mb = 0.30; // Infantry bonus multiplier for Mountain

// Helper functions for special number, perfect square, and digit sum
inline int numCharSum(int num) {
    int sum = 0;
    while (num > 9) {
        sum = 0;
        while (num) { sum += num % 10; num /= 10; }
        num = sum;
    }
    return num;
}
inline bool isPerfectSquare(int number) {
    if (number < 0) return false;
    int root = static_cast<int>(sqrt(number) + 0.5);
    return root * root == number;
}
inline bool isSpecial(int S) {
    const int primes[3] = { 3,5,7 };
    for (int k : primes) {
        int n = S;
        bool ok = true;
        while (n) {
            if (n % k > 1) { ok = false; break; }
            n /= k;
        }
        if (ok) return true;
    }
    return false;
}

enum VehicleType
{
    TRUCK,
    MORTAR,
    ANTIAIRCRAFT,
    ARMOREDCAR,
    APC,
    ARTILLERY,
    TANK
};
enum InfantryType
{
    SNIPER,
    ANTIAIRCRAFTSQUAD,
    MORTARSQUAD,
    ENGINEER,
    SPECIALFORCES,
    REGULARINFANTRY
};

class Position
{
private:
    int r, c;
public:
    Position(int r = 0, int c = 0);
    Position(const string &str_pos);
    int getRow() const;
    int getCol() const;
    void setRow(int r);
    void setCol(int c);
    string str() const;
    double dist(const Position& p) const;
};

class Unit
{
protected:
    int quantity, weight;
    Position pos;
public:
    Unit(int quantity, int weight, Position pos);
    virtual ~Unit() {}
    virtual int getAttackScore() const = 0;
    virtual bool isVehicle() = 0;
    Position getCurrentPosition() const;
    virtual string str() const = 0;
    int getQuantity() const;
    void scaleQuantity(double factor);
    void scaleWeight(double factor);
    void setWeight(int w);
};

class Vehicle : public Unit
{
private:
    VehicleType vehicleType;
public:
    Vehicle(int quantity, int weight, const Position pos, VehicleType vehicleType);
    int getAttackScore() const override;
    string str() const override;
    bool isVehicle() override ; 
    VehicleType getType() const;
};

class Infantry : public Unit
{
private:
    void applyRule();
    InfantryType infantryType;
public:
    Infantry(int quantity, int weight, const Position pos, InfantryType infantryType);
    int getAttackScore() const override;
    string str() const override;
    bool isVehicle() override ; 
    InfantryType getType() const;
};

class UnitList
{
private:
    int capacity;
    struct unitNode {
        Unit* data;
        unitNode* next;
        unitNode(Unit* u, unitNode* n = nullptr) : data(u), next(n) {}
    };
    unitNode* head;
    unitNode* tail;
    int vehicleCount, infantryCount;
    int size;
public:
    UnitList(int capacity);
    ~UnitList();
    bool insert(Unit* unit);
    bool remove(const std::vector<Unit*>& vec);
    vector<Unit*> extractAll();
    bool quantityUpdate(Unit* unit);
    bool isContain(VehicleType vehicleType);
    bool isContain(InfantryType infantryType);
    int vehicles() const;
    int infantries() const;
    string str() const;
    void clear() {
        unitNode* p = head;
        while (p) {
            unitNode* next = p->next;
            // Only delete if you own the pointer (i.e., it was allocated with new)
            // If you do not own, do not delete!
            // delete p->data; // <-- REMOVE or comment this out if you do not own the data
            delete p;
            p = next;
        }
        head = tail = nullptr;
        size = vehicleCount = infantryCount = 0;
    }
    template<typename F>
    void forEach(F f) const
    {
        for (unitNode* p = head; p; p = p->next) f(p->data);
    }
    template<typename P>
    vector<Unit*> subset(P pred) const
    {
        vector<Unit*> v;
        forEach([&](Unit* unit) { if (pred(unit)) v.push_back(unit); });
        return v;
    }
};

class Army
{
protected:
    int LF, EXP;
    string name;
    UnitList* unitList;
    BattleField* battleField;
public:
    Army(Unit** unitArray, int size, string name, BattleField* battleField);
    virtual ~Army();
    virtual void fight(Army*, bool) = 0;
    virtual string str() const = 0;
    string getName() const;
    int getLF() const;
    int getEXP() const;
    void setLF(int x);
    void setEXP(int x);
    UnitList* getUnitList() const;
    void update();
};

class LiberationArmy : public Army
{
public:
    LiberationArmy(Unit **unitArray = nullptr, int size = 0, string name = "LiberationArmy", BattleField* battleField = nullptr);
    void fight(Army *enemy, bool defense = false) override;
    string str() const override;
};

class ARVN : public Army
{
public:
    ARVN(Unit** unitArray = nullptr, int size = 0, string name = "ARVN", BattleField* battleField = nullptr);
    void fight(Army* enemy, bool defense = false) override;
    string str() const override;
};

class TerrainElement
{
public:
    TerrainElement() {}
    virtual ~TerrainElement() {}
    virtual void getEffect(class Army *army) = 0;
};

class Mountain : public TerrainElement {
private:
    Position pos;
public:
    Mountain(const Position& pos);
    void getEffect(Army* army) override;
};
class River : public TerrainElement {
private:
    Position pos;
public:
    River(const Position& pos);
    void getEffect(Army* army) override;
};
class Urban : public TerrainElement {
private:
    Position pos;
public:
    Urban(const Position& pos);
    void getEffect(Army* army) override;
};
class Fortification : public TerrainElement {
private:
    Position pos;
public:
    Fortification(const Position& pos);
    void getEffect(Army* army) override;
};
class SpecialZone : public TerrainElement {
private:
    Position pos;
public:
    SpecialZone(const Position& pos);
    void getEffect(Army* army) override;
};
class Road : public TerrainElement {
private:
    Position pos;
public:
    Road(const Position& pos);
    void getEffect(Army* army) override;
};

// Configuration class
class Configuration {
private:
    int n_rows, n_cols;
    vector<Position*> arrayForest, arrayRiver, arrayFortification, arrayUrban, arraySpecialZone;
    vector<Unit*> liberationArmyUnits, arvnUnits;
    int eventCode;
    void parseFile(const string& config_file_path);
    void parsePositions(const string& str, vector<Position*>& vec) const;
    Unit* makeUnit(const string& Token) const;
    string trim(string str) const;
    string vecPosStr(const vector<Position*>& vec) const;
    string vecUnitStr(const vector<Unit*>& vec) const;
public:
    Configuration(const string& config_file_path);
    ~Configuration();
    int getNumCols() const;
    int getNumRows() const;
    vector<Position*> getForestPositions() const;
    vector<Position*> getRiverPositions() const;
    vector<Position*> getFortificationPositions() const;
    vector<Position*> getUrbanPositions() const;
    vector<Position*> getSpecialZonePositions() const;
    vector<Unit*> getARVNUnits() const;
    vector<Unit*> getLiberationArmyUnits() const;
    int getEventCode() const;
    vector<Unit*> stealLiberation() const;
    vector<Unit*> stealARVN() const;
    string str() const;
};

// BattleField class
class BattleField {
private:
    int n_rows, n_cols;
    vector<vector<TerrainElement*>> terrainMap;
public:
    BattleField(int n_rows, int n_cols,
        const vector<Position*>& arrayForest,
        const vector<Position*>& arrayRiver,
        const vector<Position*>& arrayFortification,
        const vector<Position*>& arrayUrban,
        const vector<Position*>& arraySpecialZone);
    ~BattleField();
    void apply(Army* army);
    string str() const;
};

// HCMCampaign class
class HCMCampaign {
private:
    Configuration* config;
    BattleField* battleField;
    LiberationArmy* liberationArmy;
    ARVN* arvnArmy;
public:
    HCMCampaign(const string& config_file_path);
    ~HCMCampaign();
    void run();
    string printResult();
};

#endif // _H_HCM_CAMPAIGN_H_
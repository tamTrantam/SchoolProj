/*
 * Ho Chi Minh City University of Technology
 * Faculty of Computer Science and Engineering
 * Initial code for Assignment 2
 * Programming Fundamentals Spring 2025
 * Date: 02.02.2025
 */

// The library here is concretely set, students are not allowed to include any other libraries.
#ifndef _H_HCM_CAMPAIGN_H_
#define _H_HCM_CAMPAIGN_H_

#include "main.h"

////////////////////////////////////////////////////////////////////////
/// STUDENT'S ANSWER BEGINS HERE
/// Complete the following functions
/// DO NOT modify any parameters in the functions.
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

class UnitList
{
private:
    int capacity;
    // TODO
    struct unitNode {
        Unit* data;
        unitNode* next;
        unitNode(Unit* u, unitNode* n = nullptr) : data(u), next(n) {} //method to instantiate a node
    };
    unitNode* head; //head node
    unitNode* tail; //last node
    int vehicleCount = 0, infantryCount = 0;
    int   size;

public:
    UnitList(int capacity);
	~UnitList();
    bool insert(Unit* unit);   // return true if insert successfully
    bool remove(const std::vector<Unit*>& vec);   // return true if remove successfully
    vector<Unit*> extractAll();
    bool quantityUpdate(Unit* unit);
    bool isContain(VehicleType vehicleType);   // return true if it exists
    bool isContain(InfantryType infantryType); // return true if it exists
    int vehicles()   const;
    int infantries() const;
    string str() const;
	void clear(); // clear the list
    /*---------------- traversal helpers -------------------------------*/
    template<typename F>
    void forEach(F f) const
    {
        for (unitNode* p = head; p; p = p->next) f(p->data);
    } //apply function or lambda f to data (Unit)

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
    int LF=0, EXP=0;
    string name;
    UnitList* unitList;
    BattleField* battleField;

public:
    Army(Unit** unitArray, int size, string name, BattleField *battleField);
    virtual void fight(Army* , bool ) = 0;
    virtual string str() const = 0;
	string getName() const;
    int getLF() const;
    int getEXP() const;
    void setLF(int x);
    void setEXP(int x);
    UnitList* getUnitList() const;
    void update();
    virtual ~Army();
};


class Position
{
private:
    int r, c;
public:
    Position(int r = 0, int c = 0);
    Position(const string &str_pos); // Example: str_pos = "(1,15)"
    int getRow() const;
    int getCol() const;
    void setRow(int r);
    void setCol(int c);
    string str() const; // Example: returns "(1,15)"
    double dist(const Position& p) const;
};



class Unit
{
protected:
    int quantity, weight;
    Position pos;

public:
    Unit(int quantity, int weight, Position pos);
    virtual ~Unit() {};
    virtual int getAttackScore() const = 0;
    virtual bool isVehicle() = 0;
    Position getCurrentPosition() const;
    virtual string str() const = 0;
    int getQuantity() const;
    void scaleQuantity(double factor);
    void scaleWeight(double factor); // change parameter by multiply by factor ( disregard addition or subtraction condition)
};




class Vehicle:public Unit
{
private:
    VehicleType vehicleType;
public:
    Vehicle(int quantity, int weight, const Position pos, VehicleType vehicleType);
    int getAttackScore()const;
    string str() const;
    bool isVehicle() { return true; };
    VehicleType getType() const;
};

class Infantry :public Unit 
{
private: 
    void applyRule();
    InfantryType infantryType;
public:
    Infantry(int quantity, int weight, const Position pos, InfantryType infantryType);
    int getAttackScore() const;
    string str() const;
    bool isVehicle() { return false; };
    InfantryType getType() const;
};




class LiberationArmy : public Army
{
public:
    LiberationArmy(Unit **unitArray=nullptr, int size=0,string name ="LiberationArmy", BattleField* battleField=nullptr);
    void fight(Army *enemy, bool defense = false) override;
	string str() const override;
private:
	void construct(Unit** unitArray, int size);
	static pair<int, vector<Unit*>> calculateAttackScore(Army* enemy, bool defense);
};
class ARVN : public Army
{
public:
    ARVN(Unit** unitArray = nullptr, int size = 0, string name = "ARVN", BattleField* battleField = nullptr);
    void fight(Army* enemy, bool defense = false);
    string str() const;
private:
	void construct(Unit** unitArray, int size);

};

class TerrainElement
{
public:
    TerrainElement() {};
    ~TerrainElement() {};
    virtual void getEffect(Army *army) = 0;
};
class Mountain :public TerrainElement { private:Position pos; public:Mountain(const Position& pos); void getEffect(Army*) override; };
class River :public TerrainElement { private:Position pos; public:River(const Position& pos); void getEffect(Army*) override; };
class Urban :public TerrainElement { private:Position pos; public:Urban(const Position& pos); void getEffect(Army*)override; };
class Fortification :public TerrainElement { private:Position pos; public:Fortification(const Position& pos); void getEffect(Army*) override; };
class SpecialZone :public TerrainElement { private:Position pos; public:SpecialZone(const Position& pos); void getEffect(Army*) override; };
class Road :public TerrainElement { private:Position pos; public:Road(const Position& pos); void getEffect(Army*) override; };

class terrain 
{


    public:
    TerrainElement* element;
    Position pos;
    terrain(TerrainElement* element, const Position& pos) : element(element), pos(pos) {}
    ~terrain() { delete element; }
    void apply(Army* army) { element->getEffect(army); }
	string str() const { return pos.str(); } // Example: returns "(1,15)"
};
class BattleField
{
private:
    int n_rows, n_cols;
	vector<TerrainElement*> terrainArray; // Array of terrain elements
public:
    BattleField(int n_rows, int n_cols, vector<Position*> arrayForest,
        vector<Position*> arrayRiver, vector<Position*> arrayFortification,
        vector<Position*> arrayUrban, vector<Position*> arraySpecialZone) ;
    ~BattleField() ;
    void apply(Army* army);
        // Apply terrain effect based on the position
};

class Configuration
{
private:

    int n_rows, n_cols;
    vector<Position*> arrayForest;
    vector<Position*> arrayRiver;
    vector<Position*> arrayFortification;
    vector<Position*> arrayUrban;
	vector<Position*> arraySpecialZone;
	vector<Unit*> liberationArmyUnits;
    vector<Unit*> arvnUnits;
	int eventCode;// in range [0,99], if eventCode > 99, then last 2 digits are used
	void parseFile(const string& config_file_path);
	string trim(string str) const ; // Helper function to trim whitespace from a string
	string vecPosStr(const vector<Position*>& vec) const; // Helper function to convert vector of Position to string
	string vecUnitStr(const vector<Unit*>& vec) const; // Helper function to convert vector of Unit to string
	void parsePositions(const string& str, vector<Position*>& vec) const; // Helper function to parse positions from a line
	Unit* makeUnit(const string& Token) const; // Helper function to create a Unit from a string token

public:
	Configuration(const string& config_file_path);
	~Configuration();
    int getEventCode() const;
	int getNumRows() const;
	int getNumCols() const;
	vector<Position*> getForestPositions() const;
    vector<Position*> getRiverPositions() const;
    vector<Position*> getFortificationPositions() const;
    vector<Position*> getUrbanPositions() const;
    vector<Position*> getSpecialZonePositions() const;
	vector<Unit*> getLiberationArmyUnits() const;
	vector<Unit*> getARVNUnits() const;
	vector<Unit*> stealLiberation() const;
	vector<Unit*> stealARVN() const;
	string str() const;

};

class HCMCampaign
{
private:
    Configuration *config;
    BattleField *battleField;
    LiberationArmy *liberationArmy;
    ARVN *arvnArmy;
public:
    HCMCampaign(const string &config_file_path);
	~HCMCampaign();
    void run();
    string printResult();
};





/*-------------------------------Sub-methods & Helpers--------------------------------*/

inline int numCharSum(const int& num) //sum all character of an int till 1 digit left
{
    int sum = 0;
    string number = to_string(num);
    if (number.length() == 1)
    {
        return number[0] - '0';
    }
    else if (number.length() == 0)
    {
        return 0;
    }
    else
    {
        for (char c : number)
        {
            sum += c - '0';
        }
    }
    return numCharSum(sum);
}

inline bool isPerfectSquare(int number) //check if int is a perfect square
{
    if (number < 0) return false;
    int root = static_cast<int>(sqrt(number) + 0.5); //fix floating point number error
    return root * root == number;

}

inline bool isSpecial(int S) //check for speciality
{
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


/*-------------------------------Sub-methods & Helpers--------------------------------*/

#endif
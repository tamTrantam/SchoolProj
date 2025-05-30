#include "hcmcampaign.h"

////////////////////////////////////////////////////////////////////////
/// STUDENT'S ANSWER BEGINS HERE
////////////////////////////////////////////////////////////////////////



/*-------------------------------Position def--------------------------------*/
// Position Constructor
Position::Position(int r, int c) : r(r), c(c) {}

Position::Position(const string& str_pos) {
	sscanf(str_pos.c_str(), "(%d,%d)", &r, &c);
}

int Position::getRow() const { return r; }
int Position::getCol() const { return c; }
void Position::setRow(int _r) { r = _r; }
void Position::setCol(int _c) { c = _c; }

string Position::str() const {
	return "(" + to_string(r) + "," + to_string(c) + ")";
}

double Position::dist(const Position& p) const {
	int dr = r - p.r, dc = c - p.c;
	return sqrt(double(dr * dr + dc * dc));
}
/*-------------------------------Position def--------------------------------*/


/*-------------------------------Unit def--------------------------------*/
//Unit Constructor
Unit::Unit(int quantity, int weight, Position pos) 
{
	this->quantity = quantity;
	this->weight = weight;
	this->pos = pos;
};
Position Unit::getCurrentPosition() const { return pos; };
int Unit::getQuantity() const { return quantity; }
void Unit::scaleQuantity(double factor) { quantity = (int)ceil(quantity * factor); if (quantity < 0)quantity = 0; }
void Unit::scaleWeight(double factor){  weight = (int)ceil(weight * factor); if (weight < 0)weight = 0; }
/*-------------------------------Unit def--------------------------------*/


/*-------------------------------Vehicle def--------------------------------*/
Vehicle::Vehicle(int quantity, int weight, const Position pos, VehicleType vehicleType) : Unit(quantity, weight, pos), vehicleType(vehicleType){}

int Vehicle::getAttackScore() const
{
	return (int)ceil((vehicleType * 304 + quantity * weight) / 30.0);
};
string Vehicle::str() const 
{
	static const char* name[] = { "TRUCK","MORTAR","ANTIAIRCRAFT","ARMOREDCAR","APC","ARTILLERY","TANK" };
	ostringstream oss;
	oss << "Vehicle[vehicleType=" << name[vehicleType] // make sure the names are exact in enum
		<< ",quantity=" << quantity
		<< ",weight=" << weight
		<< ",position=" << pos.str() << "]";
	return oss.str();
};
VehicleType Vehicle:: getType() const {return vehicleType;}




/*-------------------------------Infantry def--------------------------------*/
Infantry::Infantry(int quantity, int weight, const Position pos, InfantryType infantryType) : Unit(quantity, weight, pos), infantryType(infantryType)
{
	applyRule();
}
void Infantry::applyRule()
{
	int pnum = numCharSum(getAttackScore()+1975);
	if (pnum > 7)      quantity += (int)ceil(quantity * 0.2);
	else if (pnum < 3) quantity -= (int)ceil(quantity * 0.1);
	if (quantity < 0) quantity = 0;

}
int Infantry::getAttackScore() const
{
	int score = infantryType * 56 + this->quantity * weight;

	switch (infantryType) 
	{
		case SPECIALFORCES: //consider commando case if SPECIALFORCES
		{
			if (isPerfectSquare(weight))
			{
				score += 75;
			}
			break;
		}
	}

	return score;
}
string Infantry::str() const {
	static const char* name[] = { "SNIPER","ANTIAIRCRAFTSQUAD","MORTARSQUAD","ENGINEER","SPECIALFORCES","REGULARINFANTRY" };
	ostringstream oss;
	oss << "Infantry[infantryType=" << name[infantryType] // make sure the names are exact in enum
		<< ",quantity=" << quantity
		<< ",weight=" << weight
		<< ",position=" << pos.str() << "]";
	return oss.str();
}
InfantryType Infantry:: getType() const
{
	return infantryType;
}
/*-------------------------------Infantry def--------------------------------*/


/*-------------------------------UnitList def--------------------------------*/
UnitList::UnitList(int capacity) : capacity(capacity), head(nullptr), tail(nullptr), size(0), vehicleCount(0), infantryCount(0) {}
bool UnitList::insert(Unit* unit)
{
	if (quantityUpdate(unit)) return true;                 // quantity updated
	if (vehicleCount + infantryCount >= size) return false;      // list is full

	unitNode* n = new unitNode(unit);
	if (unit->isVehicle()) {                      // PUSH‑BACK
		if (!tail) head = tail = n;
		else { tail->next = n; tail = n; }
		++vehicleCount;
	}
	else {                                   // PUSH‑FRONT
		n->next = head;
		head = n;
		if (!tail) tail = n;
		++infantryCount;
	}
	return true;
}
bool UnitList::remove(const std::vector<Unit*>& vec)
{
	for (Unit* target : vec) {
		unitNode* prev = nullptr;
		unitNode* curr = head;
		while (curr && curr->data != target) {
			prev = curr;
			curr = curr->next;
		}
		if (!curr) continue;                   // not found

		/* unlink curr */
		if (prev) prev->next = curr->next; else head = curr->next;
		if (curr == tail) tail = prev;

		(curr->data->isVehicle() ? --vehicleCount : --infantryCount);
		delete curr;                           // node only
	}
	return true;
}
vector<Unit*> UnitList::extractAll()
{
	std::vector<Unit*> out;
	for (unitNode* p = head; p; p = p->next) out.push_back(p->data);
	clear();                                   // delete nodes, reset
	return out;
}
void UnitList::clear()
{
	while (head) {
		unitNode* ptr = head->next;
		delete head; // delete the Unit object
		head = ptr; // delete the node
	}
	tail = nullptr; // reset tail
	vehicleCount = infantryCount = size = 0; // reset size
}
bool UnitList::quantityUpdate(Unit* unit)
{
	for (unitNode* ptr = head; ptr; ptr = ptr->next) //iterate over list
	{
		if (unit->isVehicle() != ptr->data->isVehicle()) continue; //skip if not same Unit type
		if (unit->isVehicle())
		{
			auto vehiclesAdded = static_cast<Vehicle*>(unit);
			auto existedVehicle = static_cast<Vehicle*>(ptr->data);
			if (vehiclesAdded)
				if (vehiclesAdded->getType() == existedVehicle->getType())
				{
					double factor = 1.0 + double(vehiclesAdded->getQuantity()) / existedVehicle->getQuantity();
					existedVehicle->scaleQuantity(factor);
					return true;
				}
		}
		else
		{
			auto infantryAdded = static_cast<Infantry*>(unit);
			auto existedInfantry = static_cast<Infantry*>(ptr->data);
			if (infantryAdded->getType() == existedInfantry->getType())
			{
				// sum quantities
				existedInfantry->scaleQuantity(1.0 + double(infantryAdded->getQuantity()) / existedInfantry->getQuantity());
				//recalc the score and PID
				int score = existedInfantry->getAttackScore(); 
				return true;
			}
		}
	}
	return false;
}
bool UnitList:: isContain(VehicleType vehicleType)
{
	for (unitNode* ptr = head; ptr; ptr = ptr->next)
	{
		if (!ptr->data->isVehicle()) continue;
		else 
		{
			auto vehiclesInList = static_cast<Vehicle*>(ptr->data);
			return vehiclesInList->getType() == vehicleType;
		}
	}
	return false; // if no vehicle of that type found
}
bool UnitList::isContain(InfantryType infantryType)
{
	for (unitNode* ptr = head; ptr; ptr = ptr->next)
	{
		if (ptr->data->isVehicle()) continue;
		else
		{
			auto infantryInList = static_cast<Infantry*>(ptr->data);
			return infantryInList->getType() == infantryType;
		}
	}
	return false;
}

int UnitList:: vehicles()   const { return vehicleCount; }
int UnitList::infantries() const { return infantryCount; }

string UnitList::str() const
{
	std::ostringstream oss;
	oss << "UnitList[count_vehicle=" << vehicleCount
		<< ";count_infantry=" << infantryCount;
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
UnitList::~UnitList()
{
	clear();
} // clear the list and delete all nodes


// ============================= BattleField =============================


Mountain::Mountain(const Position & p) : pos(p) {}
void Mountain::getEffect(Army* army) 

{
	double radius = army->getName() == "LiberationArmy" ? 2.0 : 4.0;
	double mb = army->getName() == "LiberationArmy" ? 0.30 : 0.20;
	double vp = army->getName() == "LiberationArmy" ? 0.10 : 0.05;
	int dLF = 0, dEXP = 0;
	army->getUnitList()->forEach([&](Unit* u) 
		{
		if (u->getCurrentPosition().dist(pos) <= radius + 1e-9) 
		{
			int score = u->getAttackScore();
			if (u->isVehicle()) dLF -= static_cast<int>(ceil(score * vp));
			else dEXP += static_cast<int>(ceil(score * mb));
		}
		});
	army->setLF(army->getLF() + dLF);
	army->setEXP(army->getEXP() + dEXP);
}



River::	River(const Position& p) : pos(p) {}
void River::getEffect(Army* army) 
{
	int dEXP = 0;
	army->getUnitList()->forEach([&](Unit* u) 
		{
		if (!u->isVehicle() && u->getCurrentPosition().dist(pos) <= 2.0 + 1e-9) 
		{
			dEXP -= static_cast<int>(ceil(u->getAttackScore() * 0.10));
		}
		});
	army->setEXP(army->getEXP() + dEXP);
}



Fortification::Fortification(const Position& p) : pos(p) {}
void Fortification::getEffect(Army* army) 
{
	double factor = army->getName() == "LiberationArmy" ? -0.20 : 0.20;
	int delta = 0;
	army->getUnitList()->forEach([&](Unit* u)
		{
			if (u->getCurrentPosition().dist(pos) <= 2.0 + 1e-9)
			{
				delta += static_cast<int>(ceil(u->getAttackScore() * factor));
			}
		});
	army->setLF(army->getLF() + delta);
	army->setEXP(army->getEXP() + delta);
}


SpecialZone::SpecialZone(const Position& p) : pos(p) {}
void SpecialZone::getEffect(Army* army)
{
	army->getUnitList()->forEach([&](Unit* u)
		{
			if (u->getCurrentPosition().dist(pos) <= 1.0 + 1e-9) {
				if (u->isVehicle()) army->setLF(army->getLF() - u->getAttackScore());
				else army->setEXP(army->getEXP() - u->getAttackScore());
			}
		});
}

Road::Road(const Position& p) : pos(p) {}
void Road::getEffect(Army* army)
{
	army->getUnitList()->forEach([&](Unit* u)
		{
			if (u->getCurrentPosition().dist(pos) <= 1.0 + 1e-9) {
				if (u->isVehicle()) u->scaleWeight(0.8);
				else u->scaleWeight(0.9);
			}
		});
}


Urban::Urban(const Position& p) : pos(p) {}
void Urban::getEffect(Army* army)
{
	army->getUnitList()->forEach([&](Unit* u)
		{
			double d = u->getCurrentPosition().dist(pos);
			if (army->getName() == "LiberationArmy")
			{
				if (!u->isVehicle() && d <= 5.0 + 1e-9)
				{
					Infantry* inf = dynamic_cast<Infantry*>(u);
					if (inf && (inf->getType() == SPECIALFORCES || inf->getType() == REGULARINFANTRY))
					{
						army->setEXP(army->getEXP() + static_cast<int>(ceil(2.0 * u->getAttackScore() / d)));
					}
				}
				if (u->isVehicle() && d <= 2.0 + 1e-9)
				{
					Vehicle* v = dynamic_cast<Vehicle*>(u);
					if (v && v->getType() == ARTILLERY)
					{
						army->setLF(army->getLF() - static_cast<int>(ceil(0.5 * u->getAttackScore())));
					}
				}
			}
			else {
				if (!u->isVehicle() && d <= 3.0 + 1e-9)
				{
					Infantry* inf = dynamic_cast<Infantry*>(u);
					if (inf && inf->getType() == REGULARINFANTRY)
					{
						army->setEXP(army->getEXP() + static_cast<int>(ceil(3.0 * u->getAttackScore() / (2 * d))));
					}
				}
			}
		});

};



BattleField::BattleField (int n_rows, int n_cols, vector<Position*> arrayForest, vector<Position*> arrayRiver, vector<Position*> arrayFortification, vector<Position*> arrayUrban, vector<Position*> arraySpecialZone)
	: n_rows(n_rows), n_cols(n_cols)
{
	for (Position* p : arrayForest) terrainArray.push_back(new Mountain(*p));
	for (Position* p : arrayRiver) terrainArray.push_back(new River(*p));
	for (Position* p : arrayFortification) terrainArray.push_back(new Fortification(*p));
	for (Position* p : arrayUrban) terrainArray.push_back(new Urban(*p));
	for (Position* p : arraySpecialZone) terrainArray.push_back(new SpecialZone(*p));
}

BattleField::~BattleField() 
{
	for (size_t i = 0; i < terrainArray.size(); ++i) delete terrainArray[i];
}


void BattleField::apply(Army* army) { 
    for (size_t i = 0; i < terrainArray.size(); ++i) 
        terrainArray[i]->getEffect(army); 
}
// ============================= HCMCampaign =============================

HCMCampaign::HCMCampaign(const string& config_file_path) {
	config = new Configuration(config_file_path);
	battleField = new BattleField(
		config->getNumRows(),
		config->getNumCols(),
		config->getForestPositions(),
		config->getRiverPositions(),
		config->getFortificationPositions(),
		config->getUrbanPositions(),
		config->getSpecialZonePositions()
	);

	vector<Unit*> liberationUnits = config->stealLiberation();
	vector<Unit*> arvnUnits = config->stealARVN();

	Unit** libArr = new Unit * [liberationUnits.size()];
	Unit** arvnArr = new Unit * [arvnUnits.size()];


    for (size_t i = 0; i < liberationUnits.size(); ++i) {
        libArr[i] = liberationUnits[i];
    }
    for (int i = 0; i < arvnUnits.size(); ++i) {
        arvnArr[i] = arvnUnits[i];  
    }
    liberationArmy = new LiberationArmy(libArr, static_cast<int>(liberationUnits.size()),"LiberationArmy", battleField);
	arvnArmy = new class ARVN(arvnArr, static_cast<int>(arvnUnits.size()),"ARVN", battleField);

	delete[] libArr;
	delete[] arvnArr;
}

void HCMCampaign::run() {
	battleField->apply(liberationArmy);
	battleField->apply(arvnArmy);

	int ev = config->getEventCode();
	if (ev <= 75) {
		liberationArmy->fight(arvnArmy, false);
	}
	else {
		arvnArmy->fight(liberationArmy, false);
		liberationArmy->fight(arvnArmy, false);
	}
}

string HCMCampaign::printResult() {
	ostringstream oss;
	oss << "LIBERATIONARMY[LF=" << liberationArmy->getLF()
		<< ",EXP=" << liberationArmy->getEXP()
		<< "]-ARVN[LF=" << arvnArmy->getLF()
		<< ",EXP=" << arvnArmy->getEXP() << "]";
	return oss.str();
}
HCMCampaign::~HCMCampaign() {
	delete config; delete battleField; delete liberationArmy; delete arvnArmy;
}

/*-------------------------------Army def-----------------------------------*/
Army::Army(Unit** unitArray, int size, string name, BattleField* battleField) : name(name), battleField(battleField)
{
	
	this->unitList = new UnitList(size);
	for (int i = 0; i < size; ++i) {
		unitList->insert(unitArray[i]);
	}
	this->LF = 0;
	this->EXP = 0;
}
string Army::getName() const { return name; }
int Army::getLF() const { return LF; }
int Army::getEXP() const { return EXP; }
void Army::setLF(int x) { LF = x < 0 ? 0 : x; if (LF > 1000)LF = 1000; }
void Army::setEXP(int x) { EXP = x < 0 ? 0 : x; if (EXP > 500)EXP = 500; }
UnitList* Army::getUnitList() const { return unitList; }
void Army::update() {
	int lf = 0, ex = 0;
	unitList->forEach([&](Unit* u) { (u->isVehicle() ? lf : ex) += u->getAttackScore(); });
	setLF(lf); setEXP(ex);
}
Army::~Army()
{ 
	if (unitList) delete unitList; // delete the UnitList object
	unitList = nullptr; // reset pointer
	LF = EXP = 0; // reset scores
} // destructor
/*-------------------------------Army def--------------------------------*/


// ============================= LiberationArmy =============================

LiberationArmy::LiberationArmy(Unit** unitArray, int size,string name, BattleField* battleField)
	: Army(unitArray, size, "LiberationArmy", battleField) {
	int totalLF = 0, totalEXP = 0;
	for (int i = 0; i < size; ++i) {
		if (unitArray[i]->isVehicle()) totalLF += unitArray[i]->getAttackScore();
		else totalEXP += unitArray[i]->getAttackScore();
	}
	int cap = isSpecial(totalLF + totalEXP) ? 12 : 8;
	delete this->unitList;
	unitList = new UnitList(cap);
	for (int i = 0; i < size; ++i) unitList->insert(unitArray[i]);
	LF = totalLF;
	EXP = totalEXP;
}

string LiberationArmy::str() const {
	ostringstream oss;
	oss << "LiberationArmy[LF=" << LF 
		<< ",EXP=" << EXP 
		<< ",unitList=" << unitList->str() 
		<< ",battleField=]";
	return oss.str();
}


void LiberationArmy::fight(Army* enemy, bool defense) {
	if (!defense) {
		int boostedLF = static_cast<int>(ceil(LF * 1.5));
		int boostedEXP = static_cast<int>(ceil(EXP * 1.5));

		auto inf = unitList->subset([](Unit* u) { return !u->isVehicle(); });
		auto veh = unitList->subset([](Unit* u) { return u->isVehicle(); });

		auto bestCombo = [](const vector<Unit*>& units, int threshold) {
			int n = units.size(), best = INT_MAX;
			vector<Unit*> keep;
			for (int mask = 1; mask < (1 << n); ++mask) {
				int sum = 0;
				vector<Unit*> temp;
				for (int i = 0; i < n; ++i) if (mask & (1 << i)) {
					sum += units[i]->getAttackScore();
					temp.push_back(units[i]);
				}
				if (sum > threshold && sum < best) {
					best = sum;
					keep = temp;
				}
			}
			return keep;
			};

		vector<Unit*> A = bestCombo(inf, enemy->getEXP());
		vector<Unit*> B = bestCombo(veh, enemy->getLF());

		bool gotA = !A.empty();
		bool gotB = !B.empty();

		bool victory = (gotA && gotB)
			|| (gotA && boostedLF > enemy->getLF())
			|| (gotB && boostedEXP > enemy->getEXP());

		if (victory) {
			if (gotA) unitList->remove(A);
			if (gotB) unitList->remove(B);

			if (gotA && !gotB) {
				auto allVeh = unitList->subset([](Unit* u) { return u->isVehicle(); });
				unitList->remove(allVeh);
			}
			else if (!gotA && gotB) {
				auto allInf = unitList->subset([](Unit* u) { return !u->isVehicle(); });
				unitList->remove(allInf);
			}

			auto captured = enemy->getUnitList()->extractAll();
			for (Unit* u : captured) unitList->insert(u);

			enemy->setLF(0);
			enemy->setEXP(0);
			enemy->update();
			update();
		}
		else {
			unitList->forEach([](Unit* u) { u->scaleWeight(0.9); });
			update();
		}
	}
	else {
		LF = static_cast<int>(ceil(LF * 1.3));
		EXP = static_cast<int>(ceil(EXP * 1.3));
		if (LF >= enemy->getLF() && EXP >= enemy->getEXP()) return;

		if (LF < enemy->getLF() && EXP < enemy->getEXP()) {
			auto fibUp = [](int x) {
				int a = 1, b = 1;
				while (b < x) { int t = a + b; a = b; b = t; }
				return b;
				};
			unitList->forEach([&](Unit* u) {
				int q = u->getQuantity();
				u->scaleQuantity(static_cast<double>(fibUp(q)) / q);
				});
			update();
		}
		else {
			unitList->forEach([](Unit* u) { u->scaleQuantity(0.9); });
			update();
		}
	}
}

void LiberationArmy::construct(Unit** unitArray,int size)
{
	int tLF = 0, tEXP = 0;
	for (int i = 0; i < size; i++) (unitArray[i]->isVehicle() ? tLF : tEXP) += unitArray[i]->getAttackScore();
	int cap = isSpecial(tLF + tEXP) ? 12 : 8;
	unitList = new UnitList(cap);
	for (int i = 0; i < size; ++i) unitList->insert(unitArray[i]);

	// now apply the personal‐number rule to each infantry _once_, on the merged unit:
	unitList->forEach([](Unit* u) 
		{
		if (!u->isVehicle())
		{
			auto* inf = static_cast<Infantry*>(u);
			int pnum = numCharSum(inf->getAttackScore()+1975);
			if (pnum > 7)         inf->scaleQuantity(1.2);
			else if (pnum < 3)    inf->scaleQuantity(0.9);
		}
		});
	setLF(tLF);
	setEXP(tEXP);

}


// ============================= ARVN =============================

ARVN::ARVN(Unit** unitArray, int size,string name, BattleField* battleField)
	: Army(unitArray, size, "ARVN", battleField) {
	int totalLF = 0, totalEXP = 0;
	for (int i = 0; i < size; ++i) {
		if (unitArray[i]->isVehicle()) totalLF += unitArray[i]->getAttackScore();
		else totalEXP += unitArray[i]->getAttackScore();
	}
	int cap = isSpecial(totalLF + totalEXP) ? 12 : 8;
	unitList = new UnitList(cap);
	for (int i = 0; i < size; ++i) unitList->insert(unitArray[i]);
	LF = totalLF;
	EXP = totalEXP;
}

string ARVN::str() const {
	ostringstream oss;
	oss << "ARVN[LF=" << LF 
		<< ",EXP=" << EXP 
		<< ",unitList=" << unitList->str() 
		<< ",battleField=]";
	return oss.str();
}


void ARVN::fight(Army* enemy, bool defense) {
	if (!defense) {
		unitList->forEach([](Unit* u) { u->scaleQuantity(0.8); });
		auto toRemove = unitList->subset([](Unit* u) { return u->getQuantity() <= 1; });
		unitList->remove(toRemove);
		update();
	}
	else {
		unitList->forEach([](Unit* u) { u->scaleWeight(0.8); });
		update();
	}
}
void ARVN::construct(Unit** unitArray, int size)
{
	int tLF = 0, tEXP = 0;
	for (int i = 0; i < size; i++) (unitArray[i]->isVehicle() ? tLF : tEXP) += unitArray[i]->getAttackScore();
	int cap = isSpecial(tLF + tEXP) ? 12 : 8;
	unitList = new UnitList(cap);
	for (int i = 0; i < size; i++) unitList->insert(unitArray[i]);
	// no update(): keep LF/EXP from tLF/tEXP
	setLF(tLF);
	setEXP(tEXP);
}

/*----------------------configuration def-------------------------*/
Configuration::Configuration(const string& config_file_path) 
{
	parseFile(config_file_path);
}
Configuration::~Configuration() 
{
	for (size_t i = 0; i < arrayForest.size(); ++i) delete arrayForest[i];
	for (size_t i = 0; i < arrayRiver.size(); ++i) delete arrayRiver[i];
	for (size_t i = 0; i < arrayFortification.size(); ++i) delete arrayFortification[i];
	for (size_t i = 0; i < arrayUrban.size(); ++i) delete arrayUrban[i];
	for (size_t i = 0; i < arraySpecialZone.size(); ++i) delete arraySpecialZone[i];

	for (size_t i = 0; i < liberationArmyUnits.size(); ++i) delete liberationArmyUnits[i];
	for (size_t i = 0; i < arvnUnits.size(); ++i) delete arvnUnits[i];
}
int Configuration::getNumCols() const { return n_cols; }
int Configuration::getNumRows() const { return n_rows; }
vector<Position*> Configuration::getForestPositions() const { return arrayForest; }
vector<Position*> Configuration::getRiverPositions() const { return arrayRiver; }
vector<Position*> Configuration::getFortificationPositions() const { return arrayFortification; }
vector<Position*> Configuration::getUrbanPositions() const { return arrayUrban; }
vector<Position*> Configuration::getSpecialZonePositions() const { return arraySpecialZone; }
vector<Unit*>Configuration::getARVNUnits() const  {  return arvnUnits;  }
vector<Unit*>Configuration::getLiberationArmyUnits() const  {  return liberationArmyUnits;  }
int Configuration::getEventCode() const  {  return eventCode;  }

vector<Unit*> Configuration::stealLiberation() const  
{  
    vector<Unit*> tmp = liberationArmyUnits;  
    return tmp;  
}
vector<Unit*> Configuration::stealARVN() const  
{  
	vector<Unit*> tmp = arvnUnits;  
	return tmp;  
}
string Configuration::str() const
{
	ostringstream oss;
	oss << "[num_rows=" << n_rows
		<< ",num_cols=" << n_cols
		<< ",arrayForest=" << vecPosStr(arrayForest)
		<< ",arrayRiver=" << vecPosStr(arrayRiver)
		<< ",arrayFortification=" << vecPosStr(arrayFortification)
		<< ",arrayUrban=" << vecPosStr(arrayUrban)
		<< ",arraySpecialZone=" << vecPosStr(arraySpecialZone)
		<< ",liberationArmyUnits=" << vecUnitStr(liberationArmyUnits)
		<< ",arvnUnits=" << vecUnitStr(arvnUnits)
		<< ",eventCode=" << eventCode
		<< "]";
	return oss.str();
}
string Configuration::trim(string str) const
{
	string out;
	out.reserve(str.size());
	for(char c : str) 
	{
		if (!isspace(static_cast<unsigned char>(c))) out.push_back(c);
	}
	return out;
}

string Configuration::vecPosStr(const vector<Position*>& vec) const 
{
	ostringstream oss; 
	oss << "[";
	for (size_t i = 0; i < vec.size(); ++i) {
		if (i > 0) oss << ",";
		oss << vec[i]->str();
	}
	oss << "]";
	return oss.str();
}

string Configuration::vecUnitStr(const vector<Unit*>& vec) const 
{
	ostringstream oss; 
	oss << "[";
	for (size_t i = 0; i < vec.size(); ++i) {
		if (i > 0) oss << ",";
		oss << vec[i]->str();
	}
	oss << "]";
	return oss.str();
}
void Configuration::parsePositions(const string& str, vector<Position*>& vec) const
{
	size_t p = 0;
	while ((p = str.find('(', p)) != string::npos) {
		size_t q = str.find(')', p);
		if (q == string::npos) break;
		vec.push_back(new Position(str.substr(p, q - p + 1)));
		p = q + 1;
	}
}
Unit* Configuration::makeUnit(const string& Token)const 
{
	/* 1) split NAME and "(...)" */
	size_t lp = Token.find('(');
	size_t rp = Token.find_last_of(')');
	if (lp == string::npos || rp == string::npos) return 0;
	string name = trim(Token.substr(0, lp));
	string param = Token.substr(lp, rp - lp + 1);      // keep parentheses

	/* 2) extract ALL integers inside param */
	vector<int> num; int val = 0, sign = 1; bool inNum = false;
	for (size_t i = 0; i < param.size(); ++i) {
		char ch = param[i];
		if (isdigit(ch)) {
			val = val * 10 + (ch - '0');
			inNum = true;
		}
		else {
			if (inNum) { num.push_back(sign * val); val = 0; sign = 1; inNum = false; }
			if (ch == '-') sign = -1;
		}
	}
	if (inNum) num.push_back(sign * val);          // last number

	/* need: quantity, weight, row, col, belong -> 5 numbers */
	if (num.size() != 5) return 0;
	int q = num[0], w = num[1], r = num[2], c = num[3], belong = num[4];

	/* 3) decide Vehicle vs Infantry --------------------------------*/
	static const vector<string> vehNames = {
		"TRUCK","MORTAR","ANTIAIRCRAFT","ARMOREDCAR","APC","ARTILLERY","TANK" };
	static const vector<string> infNames = {
		"SNIPER","ANTIAIRCRAFTSQUAD","MORTARSQUAD",
		"ENGINEER","SPECIALFORCES","REGULARINFANTRY" };

	for (size_t i = 0; i < vehNames.size(); ++i)
		if (vehNames[i] == name)
			return new Vehicle(q, w, Position(r, c),
				static_cast<VehicleType>(i));

	for (size_t i = 0; i < infNames.size(); ++i)
		if (infNames[i] == name)
			return new Infantry(q, w, Position(r, c),
				static_cast<InfantryType>(i));

	return 0;                                           // unknown token
}
void Configuration::parseFile(const string& config_file_path) 
{
	ifstream fin(config_file_path.c_str());
	if (!fin) throw runtime_error("Cannot open configuration file");

	string line, collected;
	auto readArrayLines = [&](string firstLine) {
		collected = firstLine;
		while (collected.find(']') == string::npos && getline(fin, line))
			collected += line;
		};

	while (getline(fin, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#') continue;

		if (line.find("NUM_ROWS=") == 0) {
			n_rows = atoi(line.substr(9).c_str());
		}
		else if (line.find("NUM_COLS=") == 0) {
			n_cols = atoi(line.substr(9).c_str());
		}
		else if (line.find("ARRAY_FOREST=[") == 0) {
			readArrayLines(line);
			parsePositions(collected, arrayForest);
		}
		else if (line.find("ARRAY_RIVER=[") == 0) {
			readArrayLines(line);
			parsePositions(collected, arrayRiver);
		}
		else if (line.find("ARRAY_FORTIFICATION=[") == 0) {
			readArrayLines(line);
			parsePositions(collected, arrayFortification);
		}
		else if (line.find("ARRAY_URBAN=[") == 0) {
			readArrayLines(line);
			parsePositions(collected, arrayUrban);
		}
		else if (line.find("ARRAY_SPECIAL_ZONE=[") == 0) {
			readArrayLines(line);
			parsePositions(collected, arraySpecialZone);
		}
		else if (line.find("EVENT_CODE=") == 0) {
			eventCode = atoi(line.substr(11).c_str()) % 100;
			if (eventCode < 0) eventCode += 100;
		}
		else if (line.find("UNIT_LIST=[") == 0) {
			/* -------- collect entire bracket content ---------- */
			readArrayLines(line);                    // fills 'collected'
			size_t bOpen = collected.find('[');
			size_t bClose = collected.rfind(']');
			if (bOpen == string::npos || bClose == string::npos) continue;
			string body = collected.substr(bOpen + 1, bClose - bOpen - 1);

			/* -------- split tokens by depth‑0 commas ----------- */
			string token;
			int depth = 0;
			auto finishToken = [&](string t) {
				t = trim(t);
				if (t.empty()) return;
				Unit* u = makeUnit(t);
				if (!u) return;
				/* last integer in token = army flag */
				int belong = -1;
				for (int i = (int)t.size() - 2; i >= 0; --i)      // look before last ')'
					if (isdigit(t[i])) { belong = t[i] - '0'; break; }
				if (belong == 0) liberationArmyUnits.push_back(u);
				else             arvnUnits.push_back(u);
				};

			for (size_t i = 0; i < body.size(); ++i) {
				char ch = body[i];
				if (ch == '(') depth++;
				if (ch == ')') depth--;
				if (ch == ',' && depth == 0) { finishToken(token); token.clear(); }
				else                         token.push_back(ch);
			}
			finishToken(token);   // last one
		}
	}
}

////////////////////////////////////////////////
/// END OF STUDENT'S ANSWER
////////////////////////////////////////////////
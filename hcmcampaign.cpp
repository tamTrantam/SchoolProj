#include "hcmcampaign.h"

// Position
Position::Position(int r, int c) : r(r), c(c) {}
Position::Position(const string& str_pos) {
    int ret = sscanf(str_pos.c_str(), "(%d,%d)", &r, &c);
    if (ret != 2) { r = 0; c = 0; }
}
int Position::getRow() const { return r; }
int Position::getCol() const { return c; }
void Position::setRow(int _r) { r = _r; }
void Position::setCol(int _c) { c = _c; }
string Position::str() const { return "(" + to_string(r) + "," + to_string(c) + ")"; }
double Position::dist(const Position& p) const {
    int dr = r - p.r, dc = c - p.c;
    return sqrt(double(dr * dr + dc * dc));
}

// Unit
Unit::Unit(int quantity, int weight, Position pos) : quantity(quantity), weight(weight), pos(pos) {}
Position Unit::getCurrentPosition() const { return pos; }
int Unit::getQuantity() const { return quantity; }
void Unit::scaleQuantity(double factor) { quantity = (int)ceil(quantity * factor); if (quantity < 0) quantity = 0; }
void Unit::scaleWeight(double factor) { weight = (int)ceil(weight * factor); if (weight < 0) weight = 0; }
void Unit::setWeight(int w) { weight = w; }

// Vehicle
Vehicle::Vehicle(int quantity, int weight, const Position pos, VehicleType vehicleType)
    : Unit(quantity, weight, pos), vehicleType(vehicleType) {}
int Vehicle::getAttackScore() const {
    return (int)vehicleType * 304 + (int)ceil((quantity * weight) / 30.0);
}
string Vehicle::str() const {
    static const char* name[] = { "TRUCK","MORTAR","ANTIAIRCRAFT","ARMOREDCAR","APC","ARTILLERY","TANK" };
    ostringstream oss;
    oss << "Vehicle[vehicleType=" << name[vehicleType]
        << ",quantity=" << quantity
        << ",weight=" << weight
        << ",position=" << pos.str() << "]";
    return oss.str();
}
VehicleType Vehicle::getType() const { return vehicleType; }
bool Vehicle::isVehicle() { return true; }

// Infantry
Infantry::Infantry(int quantity, int weight, const Position pos, InfantryType infantryType)
    : Unit(quantity, weight, pos), infantryType(infantryType) { applyRule(); }
void Infantry::applyRule() {
    int score = (int)infantryType * 56 + quantity * weight;
    if (infantryType == SPECIALFORCES && isPerfectSquare(weight)) score += 75;
    int pnum = numCharSum(score + 1975);
    if (pnum > 7) quantity += (int)ceil(quantity * 0.2);
    else if (pnum < 3) quantity -= (int)floor(quantity * 0.1);
    if (quantity < 0) quantity = 0;
}
int Infantry::getAttackScore() const {
    int score = (int)infantryType * 56 + quantity * weight;
    if (infantryType == SPECIALFORCES && isPerfectSquare(weight)) score += 75;
    return score;
}
string Infantry::str() const {
    static const char* name[] = { "SNIPER","ANTIAIRCRAFTSQUAD","MORTARSQUAD","ENGINEER","SPECIALFORCES","REGULARINFANTRY" };
    ostringstream oss;
    oss << "Infantry[infantryType=" << name[infantryType]
        << ",quantity=" << quantity
        << ",weight=" << weight
        << ",position=" << pos.str() << "]";
    return oss.str();
}
InfantryType Infantry::getType() const { return infantryType; }
bool Infantry::isVehicle() { return false; }

// UnitList
UnitList::UnitList(int capacity) : capacity(capacity), head(nullptr), tail(nullptr), size(0), vehicleCount(0), infantryCount(0) {}
UnitList::~UnitList() { clear(); }
bool UnitList::insert(Unit* unit) {
    // If same type and position exists, update quantity and discard new
    for (unitNode* ptr = head; ptr; ptr = ptr->next) {
        if (unit->isVehicle() == ptr->data->isVehicle()) {
            if (unit->isVehicle()) {
                auto v1 = static_cast<Vehicle*>(unit);
                auto v2 = static_cast<Vehicle*>(ptr->data);
                if (v1->getType() == v2->getType() && v1->getCurrentPosition().str() == v2->getCurrentPosition().str()) {
                    // Fix: add quantity directly, not by scaling
                    v2->scaleQuantity(1.0 + double(v1->getQuantity()) / v2->getQuantity());
                    delete unit;
                    return true;
                }
            } else {
                auto i1 = static_cast<Infantry*>(unit);
                auto i2 = static_cast<Infantry*>(ptr->data);
                if (i1->getType() == i2->getType() && i1->getCurrentPosition().str() == i2->getCurrentPosition().str()) {
                    i2->scaleQuantity(1.0 + double(i1->getQuantity()) / i2->getQuantity());
                    delete unit;
                    return true;
                }
            }
        }
    }
    if (vehicleCount + infantryCount >= capacity) { delete unit; return false; }
    unitNode* n = new unitNode(unit);
    if (unit->isVehicle()) {
        if (!tail) head = tail = n;
        else { tail->next = n; tail = n; }
        ++vehicleCount;
    } else {
        n->next = head;
        head = n;
        if (!tail) tail = n;
        ++infantryCount;
    }
    ++size;
    return true;
}
bool UnitList::remove(const std::vector<Unit*>& vec) {
    for (Unit* target : vec) {
        unitNode* prev = nullptr;
        unitNode* curr = head;
        while (curr && curr->data != target) {
            prev = curr;
            curr = curr->next;
        }
        if (!curr) continue;
        if (prev) prev->next = curr->next; else head = curr->next;
        if (curr == tail) tail = prev;
        if (curr->data->isVehicle()) --vehicleCount;
        else --infantryCount;
        delete curr->data;
        delete curr;
        --size;
    }
    return true;
}
vector<Unit*> UnitList::extractAll() {
    std::vector<Unit*> out;
    for (unitNode* p = head; p; p = p->next) out.push_back(p->data);
    clear();
    return out;
}
//void UnitList::clear() {
//    while (head) {
//        unitNode* ptr = head->next;
//        delete head->data;
//        delete head;
//        head = ptr;
//    }
//    tail = nullptr;
//    vehicleCount = infantryCount = size = 0;
//}
bool UnitList::quantityUpdate(Unit* unit) {
    for (unitNode* ptr = head; ptr; ptr = ptr->next) {
        if (unit->isVehicle() != ptr->data->isVehicle()) continue;
        if (unit->isVehicle()) {
            auto vehiclesAdded = static_cast<Vehicle*>(unit);
            auto existedVehicle = static_cast<Vehicle*>(ptr->data);
            if (vehiclesAdded->getType() == existedVehicle->getType()) {
                double factor = 1.0 + double(vehiclesAdded->getQuantity()) / existedVehicle->getQuantity();
                existedVehicle->scaleQuantity(factor);
                return true;
            }
        }
        else {
            auto infantryAdded = static_cast<Infantry*>(unit);
            auto existedInfantry = static_cast<Infantry*>(ptr->data);
            if (infantryAdded->getType() == existedInfantry->getType()) {
                existedInfantry->scaleQuantity(1.0 + double(infantryAdded->getQuantity()) / existedInfantry->getQuantity());
                return true;
            }
        }
    }
    return false;
}
bool UnitList::isContain(VehicleType vehicleType) {
    for (unitNode* ptr = head; ptr; ptr = ptr->next) {
        if (!ptr->data->isVehicle()) continue;
        auto vehiclesInList = static_cast<Vehicle*>(ptr->data);
        if (vehiclesInList->getType() == vehicleType) return true;
    }
    return false;
}
bool UnitList::isContain(InfantryType infantryType) {
    for (unitNode* ptr = head; ptr; ptr = ptr->next) {
        if (ptr->data->isVehicle()) continue;
        auto infantryInList = static_cast<Infantry*>(ptr->data);
        if (infantryInList->getType() == infantryType) return true;
    }
    return false;
}
int UnitList::vehicles() const { return vehicleCount; }
int UnitList::infantries() const { return infantryCount; }
string UnitList::str() const {
    ostringstream oss;
    oss << "UnitList[count_vehicle=" << vehicleCount
        << ";count_infantry=" << infantryCount;
    if (head) {
        oss << ";";
        bool first = true;
        // Infantry first
        for (unitNode* p = head; p; p = p->next) {
            if (!p->data->isVehicle()) {
                if (!first) oss << ",";
                first = false;
                oss << p->data->str();
            }
        }
        // Vehicles
        for (unitNode* p = head; p; p = p->next) {
            if (p->data->isVehicle()) {
                if (!first) oss << ",";
                first = false;
                oss << p->data->str();
            }
        }
    }
    oss << "]";
    return oss.str();
}

// TerrainElement and derived
Mountain::Mountain(const Position& p) : pos(p) {}
void Mountain::getEffect(Army* army) {
    double radius = (army->getName() == "LiberationArmy") ? 2.0 : 4.0;
    double infMul = (army->getName() == "LiberationArmy") ? 0.30 : 0.20;
    double vehMul = (army->getName() == "LiberationArmy") ? 0.10 : 0.05;
    int dLF = 0, dEXP = 0;
    army->getUnitList()->forEach([&](Unit* u) {
        if (u->getCurrentPosition().dist(pos) <= radius + 1e-9) {
            int score = u->getAttackScore();
            if (u->isVehicle()) dLF -= (int)floor(score * vehMul);
            else dEXP += (int)floor(score * infMul);
        }
    });
    army->setLF(army->getLF() + dLF);
    army->setEXP(army->getEXP() + dEXP);
}

River::River(const Position& p) : pos(p) {}
void River::getEffect(Army* army) {
    army->getUnitList()->forEach([&](Unit* u) {
        if (!u->isVehicle() && u->getCurrentPosition().dist(pos) <= 2.0 + 1e-9) {
            u->scaleWeight(0.9);
        }
    });
}

Fortification::Fortification(const Position& p) : pos(p) {}
void Fortification::getEffect(Army* army) {
    bool useCeil = (army->getName() == "ARVN");
    double factor = useCeil ? 1.20 : 0.80;
    army->getUnitList()->forEach([&](Unit* u) {
        if (u->getCurrentPosition().dist(pos) <= 2.0 + 1e-9) {
            u->scaleWeight(factor);
        }
    });
}

SpecialZone::SpecialZone(const Position& p) : pos(p) {}
void SpecialZone::getEffect(Army* army) {
    int dLF = 0, dEXP = 0;
    army->getUnitList()->forEach([&](Unit* u) {
        if (u->getCurrentPosition().dist(pos) <= 1.0 + 1e-9) {
            if (u->isVehicle()) dLF -= u->getAttackScore();
            else dEXP -= u->getAttackScore();
        }
    });
    army->setLF(army->getLF() + dLF);
    army->setEXP(army->getEXP() + dEXP);
}

Road::Road(const Position& p) : pos(p) {}
void Road::getEffect(Army* army) {
    army->getUnitList()->forEach([&](Unit* u) {
        if (u->getCurrentPosition().dist(pos) <= 1.0 + 1e-9) {
            if (u->isVehicle()) {
                u->scaleWeight(0.8);
            } else {
                u->scaleWeight(0.9);
            }
        }
    });
}

Urban::Urban(const Position& p) : pos(p) {}
void Urban::getEffect(Army* army) {
    army->getUnitList()->forEach([&](Unit* u) {
        double d = u->getCurrentPosition().dist(pos);
        if (army->getName() == "LiberationArmy") {
            if (!u->isVehicle() && d <= 5.0 + 1e-9) {
                Infantry* inf = dynamic_cast<Infantry*>(u);
                if (inf && (inf->getType() == SPECIALFORCES || inf->getType() == REGULARINFANTRY)) {
                    int add = (int)floor(2.0 * u->getAttackScore() / (d > 0 ? d : 1));
                    army->setEXP(army->getEXP() + add);
                }
            }
            if (u->isVehicle() && d <= 2.0 + 1e-9) {
                Vehicle* v = dynamic_cast<Vehicle*>(u);
                if (v && v->getType() == ARTILLERY) {
                    int sub = (int)floor(u->getAttackScore() * 0.5);
                    army->setLF(army->getLF() - sub);
                }
            }
        } else {
            if (!u->isVehicle() && d <= 3.0 + 1e-9) {
                Infantry* inf = dynamic_cast<Infantry*>(u);
                if (inf && inf->getType() == REGULARINFANTRY) {
                    int add = (int)floor(3.0 * u->getAttackScore() / (2 * (d > 0 ? d : 1)));
                    army->setEXP(army->getEXP() + add);
                }
            }
        }
    });
}

// BattleField
BattleField::BattleField(int n_rows, int n_cols,
    const vector<Position*>& arrayForest,
    const vector<Position*>& arrayRiver,
    const vector<Position*>& arrayFortification,
    const vector<Position*>& arrayUrban,
    const vector<Position*>& arraySpecialZone)
    : n_rows(n_rows), n_cols(n_cols) {
    terrainMap.assign(n_rows, vector<TerrainElement*>(n_cols, nullptr));
    for (auto* p : arrayForest) terrainMap[p->getRow()][p->getCol()] = new Mountain(*p);
    for (auto* p : arrayRiver) terrainMap[p->getRow()][p->getCol()] = new River(*p);
    for (auto* p : arrayFortification) terrainMap[p->getRow()][p->getCol()] = new Fortification(*p);
    for (auto* p : arrayUrban) terrainMap[p->getRow()][p->getCol()] = new Urban(*p);
    for (auto* p : arraySpecialZone) terrainMap[p->getRow()][p->getCol()] = new SpecialZone(*p);
}
BattleField::~BattleField() {
    for (int r = 0; r < n_rows; ++r)
        for (int c = 0; c < n_cols; ++c)
            delete terrainMap[r][c];
}
void BattleField::apply(Army* army) {
    for (int r = 0; r < n_rows; ++r)
        for (int c = 0; c < n_cols; ++c)
            if (terrainMap[r][c]) terrainMap[r][c]->getEffect(army);
}
string BattleField::str() const {
    ostringstream oss;
    oss << "BattleField[n_rows=" << n_rows << ",n_cols=" << n_cols << "]";
    return oss.str();
}

// Army
Army::Army(Unit** unitArray, int size, string name, BattleField* battleField)
    : name(name), battleField(battleField) {
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
void Army::setLF(int x) { LF = x < 0 ? 0 : x; if (LF > 1000) LF = 1000; }
void Army::setEXP(int x) { EXP = x < 0 ? 0 : x; if (EXP > 500) EXP = 500; }
UnitList* Army::getUnitList() const { return unitList; }
void Army::update() {
    int lf = 0, ex = 0;
    unitList->forEach([&](Unit* u) { (u->isVehicle() ? lf : ex) += u->getAttackScore(); });
    setLF(lf); setEXP(ex);
}
Army::~Army() { if (unitList) delete unitList; unitList = nullptr; LF = EXP = 0; }

// LiberationArmy
LiberationArmy::LiberationArmy(Unit** unitArray, int size, string name, BattleField* battleField)
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
    oss << "LiberationArmy[LF=" << LF << ",EXP=" << EXP << ",unitList=" << unitList->str() << ",battleField=]";
    return oss.str();
}
void LiberationArmy::fight(Army* enemy, bool defense) {
    // 1. Calculate total attack score for this army
    int myScore = 0;
    unitList->forEach([&](Unit* u) { myScore += u->getAttackScore(); });

    // 2. Calculate total attack score for enemy
    int enemyScore = 0;
    enemy->getUnitList()->forEach([&](Unit* u) { enemyScore += u->getAttackScore(); });

    // 3. Update LF and EXP
    int myLF = getLF() + myScore;
    int myEXP = getEXP() + myScore / 2;
    int enemyLF = enemy->getLF() - myScore;
    int enemyEXP = enemy->getEXP() - myScore / 2;
    if (enemyLF < 0) enemyLF = 0;
    if (enemyEXP < 0) enemyEXP = 0;

    setLF(myLF);
    setEXP(myEXP);
    enemy->setLF(enemyLF);
    enemy->setEXP(enemyEXP);

    // 4. Remove enemy units with quantity <= 0 (if needed)
    // (You may need to implement this logic if your rules require it)
}

// ARVN
ARVN::ARVN(Unit** unitArray, int size, string name, BattleField* battleField)
    : Army(unitArray, size, "ARVN", battleField) {
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
string ARVN::str() const {
    ostringstream oss;
    oss << "ARVN[LF=" << LF << ",EXP=" << EXP << ",unitList=" << unitList->str() << ",battleField=]";
    return oss.str();
}
void ARVN::fight(Army* enemy, bool defense) {
    // Same logic as above, but for ARVN
    int myScore = 0;
    unitList->forEach([&](Unit* u) { myScore += u->getAttackScore(); });

    int enemyScore = 0;
    enemy->getUnitList()->forEach([&](Unit* u) { enemyScore += u->getAttackScore(); });

    int myLF = getLF() + myScore;
    int myEXP = getEXP() + myScore / 2;
    int enemyLF = enemy->getLF() - myScore;
    int enemyEXP = enemy->getEXP() - myScore / 2;
    if (enemyLF < 0) enemyLF = 0;
    if (enemyEXP < 0) enemyEXP = 0;

    setLF(myLF);
    setEXP(myEXP);
    enemy->setLF(enemyLF);
    enemy->setEXP(enemyEXP);
}

// Configuration
Configuration::Configuration(const string& config_file_path) { parseFile(config_file_path); }
Configuration::~Configuration() {
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
vector<Unit*> Configuration::getARVNUnits() const { return arvnUnits; }
vector<Unit*> Configuration::getLiberationArmyUnits() const { return liberationArmyUnits; }
int Configuration::getEventCode() const { return eventCode; }
vector<Unit*> Configuration::stealLiberation() const { return liberationArmyUnits; }
vector<Unit*> Configuration::stealARVN() const { return arvnUnits; }
string Configuration::str() const {
    ostringstream oss;
    oss << "[num_rows=" << n_rows
        << ",num_cols=" << n_cols
        << ",arrayForest=" << vecPosStr(arrayForest)
        << ",arrayRiver=" << vecPosStr(arrayRiver)
        << ",arrayFortification=" << vecPosStr(arrayFortification)
        << ",arrayUrban=" << vecPosStr(arrayUrban)
        << ",arraySpecialZone=" << vecPosStr(arraySpecialZone)
        << ",liberationUnits=" << vecUnitStr(liberationArmyUnits)
        << ",ARVNUnits=" << vecUnitStr(arvnUnits)
        << ",eventCode=" << eventCode
        << "]";
    return oss.str();
}
string Configuration::trim(string str) const {
    string out;
    out.reserve(str.size());
    for (char c : str) {
        if (!isspace(static_cast<unsigned char>(c))) out.push_back(c);
    }
    return out;
}
string Configuration::vecPosStr(const vector<Position*>& vec) const {
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << ",";
        oss << vec[i]->str();
    }
    oss << "]";
    return oss.str();
}
string Configuration::vecUnitStr(const vector<Unit*>& vec) const {
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) oss << ",";
        oss << vec[i]->str();
    }
    oss << "]";
    return oss.str();
}
void Configuration::parsePositions(const string& str, vector<Position*>& vec) const {
    size_t p = 0;
    while ((p = str.find('(', p)) != string::npos) {
        size_t q = str.find(')', p);
        if (q == string::npos) break;
        vec.push_back(new Position(str.substr(p, q - p + 1)));
        p = q + 1;
    }
}
Unit* Configuration::makeUnit(const string& Token) const {
    size_t lp = Token.find('(');
    size_t rp = Token.find_last_of(')');
    if (lp == string::npos || rp == string::npos) return nullptr;
    string name = trim(Token.substr(0, lp));
    string param = Token.substr(lp + 1, rp - lp - 1);

    vector<int> num;
    int val = 0, sign = 1; bool inNum = false;
    for (size_t i = 0; i < param.size(); ++i) {
        char ch = param[i];
        if (isdigit(ch)) {
            val = val * 10 + (ch - '0');
            inNum = true;
        } else {
            if (inNum) { num.push_back(sign * val); val = 0; sign = 1; inNum = false; }
            if (ch == '-') sign = -1;
        }
    }
    if (inNum) num.push_back(sign * val);

    if (num.size() != 5) return nullptr;
    int q = num[0], w = num[1], r = num[2], c = num[3], belong = num[4];

    static const vector<string> vehNames = {
        "TRUCK","MORTAR","ANTIAIRCRAFT","ARMOREDCAR","APC","ARTILLERY","TANK"
    };
    static const vector<string> infNames = {
        "SNIPER","ANTIAIRCRAFTSQUAD","MORTARSQUAD",
        "ENGINEER","SPECIALFORCES","REGULARINFANTRY"
    };

    for (size_t i = 0; i < vehNames.size(); ++i)
        if (vehNames[i] == name)
            return new Vehicle(q, w, Position(r, c), static_cast<VehicleType>(i));

    for (size_t i = 0; i < infNames.size(); ++i)
        if (infNames[i] == name)
            return new Infantry(q, w, Position(r, c), static_cast<InfantryType>(i));

    return nullptr;
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
            readArrayLines(line);
            size_t bOpen = collected.find('[');
            size_t bClose = collected.rfind(']');
            if (bOpen == string::npos || bClose == string::npos) continue;
            string body = collected.substr(bOpen + 1, bClose - bOpen - 1);

            string token;
            int depth = 0;
            auto finishToken = [&](string t) {
                t = trim(t);
                if (t.empty()) return;
                Unit* u = makeUnit(t);
                if (!u) return;
                int belong = -1;
                for (int i = (int)t.size() - 2; i >= 0; --i)
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
            finishToken(token);
        }
    }
}

// HCMCampaign
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
    Unit** libArr = nullptr;
    Unit** arvnArr = nullptr;
    size_t libSize = liberationUnits.size();
    size_t arvnSize = arvnUnits.size();
    if (libSize > 0) {
        libArr = new Unit*[libSize];
        for (size_t i = 0; i < libSize; ++i) libArr[i] = liberationUnits[i];
    }
    if (arvnSize > 0) {
        arvnArr = new Unit*[arvnSize];
        for (size_t i = 0; i < arvnSize; ++i) arvnArr[i] = arvnUnits[i];
    }
    liberationArmy = new LiberationArmy(libArr, static_cast<int>(libSize), "LiberationArmy", battleField);
    arvnArmy = new ARVN(arvnArr, static_cast<int>(arvnSize), "ARVN", battleField);
    if (libArr) delete[] libArr;
    if (arvnArr) delete[] arvnArr;
}
HCMCampaign::~HCMCampaign() {
    delete config; delete battleField; delete liberationArmy; delete arvnArmy;
}
void HCMCampaign::run() {
    battleField->apply(liberationArmy);
    battleField->apply(arvnArmy);
    int ev = config->getEventCode();
    if (ev <= 75) {
        liberationArmy->fight(arvnArmy, false);
    } else {
        arvnArmy->fight(liberationArmy, false);
        liberationArmy->fight(arvnArmy, true);
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
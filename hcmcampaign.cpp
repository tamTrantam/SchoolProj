#include "hcmcampaign.h"

// --- Terrain Effects ---
void Mountain::getEffect(Army* a) {
    double rad = a->isLiberation() ? 2.0 : 4.0, mb = a->isLiberation() ? 0.30 : 0.20, vp = a->isLiberation() ? 0.10 : 0.05;
    int dLF = 0, dEXP = 0;
    a->units()->forEach([&](Unit* u) {
        double d = u->getCurrentPosition().dist(pos); if (d > rad + 1e-9) return;
        int sc = u->getAttackScore();
        if (u->isVehicle()) dLF -= (int)ceil(sc * vp); else dEXP += (int)ceil(sc * mb);
        });
    a->setLF(a->getLF() + dLF); a->setEXP(a->getEXP() + dEXP);
}
void River::getEffect(Army* a) {
    int dEXP = 0;
    a->units()->forEach([&](Unit* u) {
        if (!u->isVehicle() && u->getCurrentPosition().dist(pos) <= 2.0 + 1e-9)
            dEXP -= (int)ceil(u->getAttackScore() * 0.10);
        });
    a->setEXP(a->getEXP() + dEXP);
}
void Urban::getEffect(Army* a) {
    if (a->isLiberation()) {
        a->units()->forEach([&](Unit* u) {
            double d = u->getCurrentPosition().dist(pos);
            if (!u->isVehicle() && d <= 5.0 + 1e-9) {
                Infantry* inf = static_cast<Infantry*>(u);
                if (inf->getType() == SPECIALFORCES || inf->getType() == REGULARINFANTRY) {
                    a->setEXP(a->getEXP() + (int)ceil(2.0 * u->getAttackScore() / (d > 0 ? d : 1)));
                }
            }
            if (u->isVehicle() && d <= 2.0 + 1e-9) {
                Vehicle* v = static_cast<Vehicle*>(u);
                if (v->getType() == ARTILLERY) a->setLF(a->getLF() - (int)ceil(0.5 * u->getAttackScore()));
            }
            });
    }
    else {
        a->units()->forEach([&](Unit* u) {
            if (!u->isVehicle() && u->getCurrentPosition().dist(pos) <= 3.0 + 1e-9) {
                Infantry* inf = static_cast<Infantry*>(u);
                if (inf->getType() == REGULARINFANTRY) {
                    double d = u->getCurrentPosition().dist(pos);
                    a->setEXP(a->getEXP() + (int)ceil(3.0 * u->getAttackScore() / (2 * (d > 0 ? d : 1))));
                }
            }
            });
    }
}
void Fortification::getEffect(Army* a) {
    double rad = 2.0, f = a->isLiberation() ? -0.20 : 0.20; int delta = 0;
    a->units()->forEach([&](Unit* u) { if (u->getCurrentPosition().dist(pos) <= rad + 1e-9) delta += (int)ceil(u->getAttackScore() * f); });
    a->setLF(a->getLF() + delta); a->setEXP(a->getEXP() + delta);
}
void SpecialZone::getEffect(Army* a) {
    a->units()->forEach([&](Unit* u) {
        if (u->getCurrentPosition().dist(pos) <= 1.0 + 1e-9) {
            if (u->isVehicle()) a->setLF(a->getLF() - u->getAttackScore());
            else               a->setEXP(a->getEXP() - u->getAttackScore());
        }
        });
}

// --- Army Build ---
void LiberationArmy::build(Unit** arr, int sz) {
    int tLF = 0, tEXP = 0;
    for (int i = 0; i < sz; i++) (arr[i]->isVehicle() ? tLF : tEXP) += arr[i]->getAttackScore();
    int cap = isSpecialNumber(tLF + tEXP) ? 12 : 8;
    list = new UnitList(cap);
    for (int i = 0; i < sz; ++i) list->insert(arr[i]);
    list->forEach([](Unit* u) {
        if (!u->isVehicle()) {
            auto* inf = static_cast<Infantry*>(u);
            int pnum = personalNumber(inf->getAttackScore());
            if (pnum > 7)       inf->scaleQuantity(1.2);
            else if (pnum < 3)  inf->scaleQuantity(0.9);
        }
        });
    setLF(tLF);
    setEXP(tEXP);
}
pair<int, vector<Unit*> > LiberationArmy::bestCombo(const vector<Unit*>& v, int need) {
    int n = v.size(), best = INT_MAX; vector<Unit*> keep;
    for (int mask = 1; mask < (1 << n); ++mask) {
        int sum = 0; vector<Unit*> tmp;
        for (int i = 0; i < n; ++i) if (mask >> i & 1) { sum += v[i]->getAttackScore(); tmp.push_back(v[i]); }
        if (sum > need && sum < best) { best = sum; keep = tmp; }
    }
    return make_pair(best, keep);
}
void ARVN::build(Unit** arr, int sz) {
    int tLF = 0, tEXP = 0;
    for (int i = 0; i < sz; i++) (arr[i]->isVehicle() ? tLF : tEXP) += arr[i]->getAttackScore();
    int cap = isSpecialNumber(tLF + tEXP) ? 12 : 8;
    list = new UnitList(cap);
    for (int i = 0; i < sz; i++) list->insert(arr[i]);
    setLF(tLF);
    setEXP(tEXP);
}

// --- Army Fight Logic ---
void LiberationArmy::fight(Army* enemy, bool defense) {
    if (!defense) {
        int myLF = (int)ceil(LF * 1.5), myEXP = (int)ceil(EXP * 1.5);
        auto inf = units()->subset([](Unit* u) { return !u->isVehicle(); });
        auto veh = units()->subset([](Unit* u) { return u->isVehicle(); });
        auto A = bestCombo(inf, enemy->getEXP());
        auto B = bestCombo(veh, enemy->getLF());
        bool gotA = !A.second.empty(), gotB = !B.second.empty();
        bool victory = false;
        if (gotA && gotB)
            victory = true;
        else if (gotA && myLF > enemy->getLF())
            victory = true;
        else if (gotB && myEXP > enemy->getEXP())
            victory = true;
        if (victory) {
            if (gotA) units()->remove(A.second);
            if (gotB) units()->remove(B.second);
            if (gotA && !gotB) {
                auto allV = units()->subset([](Unit* u) { return u->isVehicle(); });
                units()->remove(allV);
            }
            else if (!gotA && gotB) {
                auto allI = units()->subset([](Unit* u) { return !u->isVehicle(); });
                units()->remove(allI);
            }
            auto captured = enemy->units()->extractAll();
            for (Unit* u : captured)
                units()->insert(u);
            enemy->setLF(0);
            enemy->setEXP(0);
            enemy->update();
            update();
        }
        else {
            units()->forEach([](Unit* u) { u->scaleWeight(0.9); });
            update();
        }
    }
    else {
        setLF((int)ceil(LF * 1.3));
        setEXP((int)ceil(EXP * 1.3));
        if (LF >= enemy->getLF() && EXP >= enemy->getEXP())
            return;
        if (LF < enemy->getLF() && EXP < enemy->getEXP()) {
            auto fibUp = [](int x) {
                int a = 1, b = 1;
                while (b < x) { int t = a + b; a = b; b = t; }
                return b;
                };
            units()->forEach([&](Unit* u) {
                int q = u->getQuantity();
                u->scaleQuantity((fibUp(q) + 0.0) / q);
                });
            update();
        }
        else {
            units()->forEach([](Unit* u) { u->scaleQuantity(0.9); });
            update();
        }
    }
}
void ARVN::fight(Army* enemy, bool defense) {
    if (!defense) {
        units()->forEach([](Unit* u) { u->scaleQuantity(0.8); });
        vector<Unit*> del = units()->subset([](Unit* u) { return u->getQuantity() <= 1; });
        units()->remove(del); update();
    }
    else {
        units()->forEach([](Unit* u) { u->scaleWeight(0.8); }); update();
    }
}

// --- Configuration ---
Configuration::~Configuration() {
    for (size_t i = 0; i < arrayForest.size(); ++i) delete arrayForest[i];
    for (size_t i = 0; i < arrayRiver.size(); ++i) delete arrayRiver[i];
    for (size_t i = 0; i < arrayFortification.size(); ++i) delete arrayFortification[i];
    for (size_t i = 0; i < arrayUrban.size(); ++i) delete arrayUrban[i];
    for (size_t i = 0; i < arraySpecialZone.size(); ++i) delete arraySpecialZone[i];
    for (size_t i = 0; i < liberationUnits.size(); ++i) delete liberationUnits[i];
    for (size_t i = 0; i < ARVNUnits.size(); ++i) delete ARVNUnits[i];
}
string Configuration::str() const {
    ostringstream oss;
    oss << "[num_rows=" << num_rows
        << ",num_cols=" << num_cols
        << ",arrayForest=" << vecPosStr(arrayForest)
        << ",arrayRiver=" << vecPosStr(arrayRiver)
        << ",arrayFortification=" << vecPosStr(arrayFortification)
        << ",arrayUrban=" << vecPosStr(arrayUrban)
        << ",arraySpecialZone=" << vecPosStr(arraySpecialZone)
        << ",liberationUnits=" << vecUnitStr(liberationUnits)
        << ",ARVNUnits=" << vecUnitStr(ARVNUnits)
        << ",eventCode=" << eventCode
        << "]";
    return oss.str();
}
std::string Configuration::trim(std::string s) {
    std::string out;
    out.reserve(s.size());
    for (char ch : s)
        if (!std::isspace(static_cast<unsigned char>(ch)))
            out.push_back(ch);
    return out;
}
string Configuration::vecPosStr(const vector<Position*>& v) {
    ostringstream oss; oss << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) oss << ",";
        oss << v[i]->str();
    }
    oss << "]";
    return oss.str();
}
string Configuration::vecUnitStr(const vector<Unit*>& v) {
    ostringstream oss; oss << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) oss << ",";
        oss << v[i]->str();
    }
    oss << "]";
    return oss.str();
}
void Configuration::parsePosArray(const string& raw, vector<Position*>& dst) {
    size_t p = 0;
    while ((p = raw.find('(', p)) != string::npos) {
        size_t q = raw.find(')', p);
        if (q == string::npos) break;
        dst.push_back(new Position(raw.substr(p, q - p + 1)));
        p = q + 1;
    }
}
Unit* Configuration::makeUnit(const string& rawToken) {
    size_t lp = rawToken.find('(');
    size_t rp = rawToken.find_last_of(')');
    if (lp == string::npos || rp == string::npos) return 0;
    string name = trim(rawToken.substr(0, lp));
    string param = rawToken.substr(lp, rp - lp + 1);
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
    if (inNum) num.push_back(sign * val);
    if (num.size() != 5) return 0;
    int q = num[0], w = num[1], r = num[2], c = num[3], belong = num[4];
    static const vector<string> vehNames = {
        "TRUCK","MORTAR","ANTIAIRCRAFT","ARMOREDCAR","APC","ARTILLERY","TANK" };
    static const vector<string> infNames = {
        "SNIPER","ANTIAIRCRAFTSQUAD","MORTARSQUAD",
        "ENGINEER","SPECIALFORCES","REGULARINFANTRY" };
    for (size_t i = 0; i < vehNames.size(); ++i)
        if (vehNames[i] == name)
            return new Vehicle(q, w, Position(r, c), static_cast<VehicleType>(i));
    for (size_t i = 0; i < infNames.size(); ++i)
        if (infNames[i] == name)
            return new Infantry(q, w, Position(r, c), static_cast<InfantryType>(i));
    return 0;
}
void Configuration::parseFile(const string& path) {
    ifstream fin(path.c_str());
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
            num_rows = atoi(line.substr(9).c_str());
        }
        else if (line.find("NUM_COLS=") == 0) {
            num_cols = atoi(line.substr(9).c_str());
        }
        else if (line.find("ARRAY_FOREST=[") == 0) {
            readArrayLines(line);
            parsePosArray(collected, arrayForest);
        }
        else if (line.find("ARRAY_RIVER=[") == 0) {
            readArrayLines(line);
            parsePosArray(collected, arrayRiver);
        }
        else if (line.find("ARRAY_FORTIFICATION=[") == 0) {
            readArrayLines(line);
            parsePosArray(collected, arrayFortification);
        }
        else if (line.find("ARRAY_URBAN=[") == 0) {
            readArrayLines(line);
            parsePosArray(collected, arrayUrban);
        }
        else if (line.find("ARRAY_SPECIAL_ZONE=[") == 0) {
            readArrayLines(line);
            parsePosArray(collected, arraySpecialZone);
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
                if (belong == 0) liberationUnits.push_back(u);
                else             ARVNUnits.push_back(u);
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

// --- HCMCampaign ---
HCMCampaign::HCMCampaign(const string& path) {
    cfg = new Configuration(path);
    bf = new BattleField(
        cfg->getNumRows(),
        cfg->getNumCols(),
        cfg->getForestPositions(),
        cfg->getRiverPositions(),
        cfg->getFortificationPositions(),
        cfg->getUrbanPositions(),
        cfg->getSpecialZonePositions()
    );
    vector<Unit*> libVec = cfg->stealLiberation();
    vector<Unit*> arvnVec = cfg->stealARVN();
    auto makeArr = [](const vector<Unit*>& v) {
        Unit** a = new Unit * [v.size()];
        for (size_t i = 0; i < v.size(); ++i) a[i] = v[i];
        return a;
        };
    Unit** libArr = makeArr(libVec);
    Unit** arArr = makeArr(arvnVec);
    lib = new LiberationArmy(libArr, libVec.size(), bf);
    arvn = new ARVN(arArr, arvnVec.size(), bf);
    delete[] libArr;
    delete[] arArr;
}
HCMCampaign::~HCMCampaign() { delete cfg; delete bf; delete lib; delete arvn; }
void HCMCampaign::run() {
    bf->apply(lib);
    bf->apply(arvn);
    int ev = cfg->getEventCode();
    if (ev <= 75) {
        lib->fight(arvn, false);
    }
    else {
        arvn->fight(lib, false);
        lib->fight(arvn, false);
    }
}
string HCMCampaign::printResult() const {
    ostringstream oss; oss << "LIBERATIONARMY[LF=" << lib->getLF() << ",EXP=" << lib->getEXP() << "]-"
        << "ARVN[LF=" << arvn->getLF() << ",EXP=" << arvn->getEXP() << "]";
    return oss.str();
}
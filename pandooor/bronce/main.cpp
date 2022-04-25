#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>

using namespace std;

class Entity {
public:
    int id;
    int type;
    int x, y;
    int shield_life;
    int is_controlled;
    int health;
    int vx, vy;
    int near_base;
    int threat_for;
    int distance_to_base;
    int distance_to_hero;

    int calculate_distance_to_target(int target_x, int target_y) {
        cerr << "calculate threat level for id" << id << endl;
        int distance_to_target = sqrt(pow(target_x - x, 2) + pow(target_y - y, 2));
        cerr << "distance_to_target is: " << distance_to_base << endl;
        return distance_to_target;
    }
};

int main() {
    int base_x; // The corner of the map representing your base
    int base_y;
    cin >> base_x >> base_y;
    bool tlp = (base_x == 0)? true : false;
    cin.ignore();
    int heroes_per_player; // Always 3
    cin >> heroes_per_player;
    cin.ignore();
    bool up = true;
    bool reached_start_position = false;

    // game loop
    while (1) {
        int myHealth;
        int myMana;
        int oppHealth;
        int oppMana;
        int entity_count; // Amount of heros and monsters you can see
        cin >> myHealth >> myMana >> oppHealth >> oppMana >> entity_count;
        cin.ignore();

        std::vector<Entity> heroes;
        std::vector<Entity> opp_heroes;
        std::vector<Entity> monsters;

        for (int i = 0; i < entity_count; i++) {
            int id; // Unique identifier
            int type; // 0=monster, 1=your hero, 2=opponent hero
            int x; // Position of this entity
            int y;
            int shield_life; // Ignore for this league; Count down until shield spell fades
            int is_controlled; // Ignore for this league; Equals 1 when this entity is under a control spell
            int health; // Remaining health of this monster
            int vx; // Trajectory of this monster
            int vy;
            int near_base; // 0=monster with no target yet, 1=monster targeting a base
            int threat_for; // Given this monster's trajectory, is it a threat to 1=your base, 2=your opponent's base, 0=neither
            cin >> id >> type >> x >> y >> shield_life >> is_controlled >> health >> vx >> vy >> near_base
                >> threat_for;
            cin.ignore();

            Entity entity;
            entity.id = id;
            entity.type = type;
            entity.x = x;
            entity.y = y;
            entity.shield_life = shield_life;
            entity.is_controlled = is_controlled;
            entity.health = health;
            entity.vx = vx;
            entity.vy = vy;
            entity.near_base = near_base;
            entity.threat_for = threat_for;

            switch (entity.type) {
                case 0:
                    monsters.push_back(entity);
                    break;
                case 1:
                    heroes.push_back(entity);
                    break;
                case 2:
                    opp_heroes.push_back(entity);
                    break;
            }
        }

        cerr << "base_x" << base_x << endl;
        cerr << "base_y" << base_y << endl;
        cerr << "boolean tlp" << tlp << endl;
        cerr << "finished input" << endl;
        cerr << "heroes array size is: " << heroes.size() << endl;
        cerr << "opp_heroes array size is: " << opp_heroes.size() << endl;
        cerr << "monsters array size is: " << monsters.size() << endl;

        std::list<Entity> defender_one_targets;
        std::list<Entity> defender_two_targets;
        std::list<Entity> aggressor_targets;

        for (int i = 0; i < monsters.size(); i++) {
            Entity monster = monsters.at(i);
            monster.distance_to_base = monster.calculate_distance_to_target(base_x, base_y);
            int distance_defender_one = monster.calculate_distance_to_target(heroes.at(0).x, heroes.at(0).y);
            int distance_defender_two = monster.calculate_distance_to_target(heroes.at(1).x, heroes.at(1).y);
            int distance_aggressor = monster.calculate_distance_to_target(heroes.at(2).x, heroes.at(2).y);
            if (distance_defender_one < distance_defender_two) {
                if (distance_defender_one < distance_aggressor) {
                    monster.distance_to_hero = distance_defender_one;
                    defender_one_targets.push_back(monster);
                    continue;
                }
            } else {
                if (distance_defender_two < distance_aggressor) {
                    monster.distance_to_hero = distance_defender_two;
                    defender_two_targets.push_back(monster);
                    continue;
                }
            }
            monster.distance_to_hero = distance_aggressor;
            aggressor_targets.push_back(monster);
        }

        cerr << "monsters array size for hero 0 is: " << defender_one_targets.size() << endl;
        cerr << "monsters array size for hero 1 is: " << defender_two_targets.size() << endl;
        cerr << "monsters array size for hero 2 is: " << aggressor_targets.size() << endl;

        defender_one_targets.sort(
                [](const Entity &a, const Entity &b) { return a.distance_to_base < b.distance_to_base; });
        defender_two_targets.sort(
                [](const Entity &a, const Entity &b) { return a.distance_to_base < b.distance_to_base; });
        aggressor_targets.sort(
                [](const Entity &a, const Entity &b) { return a.distance_to_hero < b.distance_to_hero; });

        //upper defender
        //  patrol between x = 4500, y = 4500 and x = 6000, y = 800
        Entity target;
        if (!defender_one_targets.empty()) {
            target = defender_one_targets.front();
            cerr << "target monsters id: " << target.id << " and distance_to_base: " << target.distance_to_base << endl;
            if (target.distance_to_base < 800) {
                cout << "SPELL WIND " << ((tlp) ? 4500 : base_x - 4500) << " " << ((tlp) ? 4500 : base_y - 4500) << endl;
            } else {
                cout << "MOVE " << target.x << " " << target.y << endl;
            }
        } else {
            if (!reached_start_position && heroes.at(0).x == ((tlp) ? 5200 : base_x - 5200)) {
                reached_start_position = true;
            }
            if (reached_start_position) {
                if (heroes.at(0).x == ((tlp) ? 6000 : base_x - 6000) && heroes.at(0).y == ((tlp) ? 800 : base_y - 800)) {
                    up = false;
                }
                if (heroes.at(0).x == ((tlp) ? 4500 : base_x - 4500) && heroes.at(0).y == ((tlp) ? 4500 : base_y - 4500)) {
                    up = true;
                }
                if (up) {
                    cout << "MOVE " << ((tlp) ? 6000 : base_x - 6000) << " " << ((tlp) ? 800 : base_y - 800) << endl;
                } else {
                    cout << "MOVE " << ((tlp) ? 4500 : base_x - 4500) << " " << ((tlp) ? 4500 : base_y - 4500) << endl;
                }
            } else {
                cout << "MOVE " << ((tlp) ? 5200 : base_x - 5200) << " " << ((tlp) ? 2700 : base_y - 2700) << endl;
            }
        }

        //lower defender
        //  patrol between x = 4500, y = 4500 and x = 800, y = 6000
        if (!defender_two_targets.empty()) {
            target = defender_two_targets.front();
            cerr << "target monsters id: " << target.id << " and distance_to_base: " << target.distance_to_base << endl;
            if (target.distance_to_base < 800) {
                cout << "SPELL WIND " << ((tlp) ? 4500 : base_x - 4500) << " " << ((tlp) ? 4500 : base_y - 4500) << endl;
            } else {
                cout << "MOVE " << target.x << " " << target.y << endl;
            }
        } else {
            if (reached_start_position) {
                if (heroes.at(1).x == ((tlp) ? 4500 : base_x - 4500) && heroes.at(1).y == ((tlp) ? 4500 : base_y - 4500)) {
                    up = false;
                }
                if (heroes.at(1).x == ((tlp) ? 800 : base_x - 800) && heroes.at(1).y == ((tlp) ? 6000 : base_y - 6000)) {
                    up = true;
                }
                if (up) {
                    cout << "MOVE " << ((tlp) ? 4500 : base_x - 4500) << " " << ((tlp) ? 4500 : base_y - 4500) << endl;
                } else {
                    cout << "MOVE " << ((tlp) ? 800 : base_x - 800) << " " << ((tlp) ? 6000 : base_y - 6000) << endl;
                }
            } else {
                cout << "MOVE " << ((tlp) ? 2700 : base_x - 2700) << " " << ((tlp) ? 5200 : base_y - 5200) << endl;
            }
        }

        //aggressor
        //  TODO: change patrol pattern
        if (!aggressor_targets.empty()) {
            //TODO: don't attack on the enemy side of the map (helping the other team)
            target = aggressor_targets.front();
            cerr << "target monsters id: " << target.id << " and distance_to_base: " << target.distance_to_base << endl;
            if (target.distance_to_base < 800) {
                cout << "SPELL WIND " << ((tlp) ? 4500 : base_x - 4500) << " " << ((tlp) ? 4500 : base_y - 4500) << endl;
            } else {
                cout << "MOVE " << target.x << " " << target.y << endl;
            }
        } else {
            if (reached_start_position) {
                if (up) {
                    cout << "MOVE " << ((tlp) ? 11000 : base_x - 11000) << " " << ((tlp) ? 1500 : base_y - 1500) << endl;
                } else {
                    cout << "MOVE " << ((tlp) ? 5500 : base_x - 5500) << " " << ((tlp) ? 7300 : base_y - 7300) << endl;
                }
            } else {
                cout << "MOVE " << ((tlp) ? 4500 : base_x - 4500) << " " << ((tlp) ? 4500 : base_y - 4500) << endl;
            }
        }
    }
}


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
        int distance_to_target = sqrt(pow(target_x - x, 2) + pow(target_y - y, 2));
        return distance_to_target;
    }

    Entity copy() {
        Entity copy;
        copy.id = id;
        copy.type = type;
        copy.x = x;
        copy.y = y;
        copy.shield_life = shield_life;
        copy.is_controlled = is_controlled;
        copy.health = health;
        copy.vx = vx;
        copy.vy = vy;
        copy.near_base = near_base;
        copy.threat_for = threat_for;
        copy.distance_to_base = distance_to_base;
        copy.distance_to_hero = distance_to_hero;
        return copy;
    }
};

class Hero_States {
public:
    bool forward = true;
    int home_x;
    int home_y;
    int enemy_x;
    int enemy_y;
    int patrol_start_x;
    int patrol_end_x;
    int patrol_start_y;
    int patrol_end_y;
    int defend_base_x;
    int defend_base_y;
    int hero_role; // 1 = defender, 2 = farmer, 3 = attacker
    int hero_id;
    int send_count = 0;
    int shielded_monsters = 0;
    bool enemy_takes_control_over_hero = false;
    bool enemy_hero_in_range_to_control;
    Entity hero;
    std::list<Entity> targets;
    std::vector<Entity> opp_heroes;

    void calculate_positions(int base_x, int base_y, int start_x, int start_y, int end_x, int end_y) {
        home_x = base_x;
        home_y = base_y;

        if (base_x == 0) {
            patrol_start_x = start_x;
            patrol_end_x = end_x;
            patrol_start_y = start_y;
            patrol_end_y = end_y;
            defend_base_x = 4500;
            defend_base_y = 4500;
            enemy_x = 17630;
            enemy_y = 9000;
        } else {
            patrol_start_x = base_x - start_x;
            patrol_end_x = base_x - end_x;
            patrol_start_y = base_y - start_y;
            patrol_end_y = base_y - end_y;
            defend_base_x = base_x - 4500;
            defend_base_y = base_y - 4500;
            enemy_x = 0;
            enemy_y = 0;
        }
    }

    void move() {
        //detect enemy taking control over my defenders
        if (!enemy_takes_control_over_hero && hero.is_controlled) {
            enemy_takes_control_over_hero = true;
        }

        if (targets.empty()) {
            patrol();
        } else {
            if (hero_role == 3) {
                if (enemy_takes_control_over_hero && (hero.shield_life == 0) && enemy_hero_in_range_to_control) {
                    cout << "SPELL SHIELD " << hero_id << " " << hero_id << endl;
                } else {
                    if (send_count < 15) {
                        send();
                    } else {
                        disturb_enemies();
                    }
                }
            } else {
                if (enemy_takes_control_over_hero && (hero.shield_life == 0) && enemy_hero_in_range_to_control) {
                    cout << "SPELL SHIELD " << hero_id << " " << hero_id << endl;
                } else {
                    Entity target;
                    target = targets.front();
                    if ((target.distance_to_base < 1200) &&
                        (target.shield_life == 0)) { //changed from 800 which is too low probably
                        defend_base_wind();
                    } else {
                        attack(target);
                    }
                }
            }
        }
    }

    void send() {
        int iterations = targets.size();
        for (int i = 0; i < iterations; i++) {
            if (targets.front().threat_for != 2 &&
                (targets.front().calculate_distance_to_target(hero.x, hero.y) < 2200)) {
                //TODO: send to both edges of the enemy cycle
                cout << "SPELL CONTROL " << targets.front().id << " " << enemy_x << " " << enemy_y << " "
                     << hero_id << endl;
                send_count++;
                return;
            } else {
                targets.pop_front();
            }
        }
        patrol();
    }

    void disturb_enemies() {
        //TODO: check holding position to engage enemy
        int goal_pos_x = 2500;
        int goal_pos_y = 2500;
        if (home_x == 0) {
            goal_pos_x = enemy_x - 2500;
            goal_pos_y = enemy_y - 2500;
        }
        //TODO: check algo sequence when first try not success
        if (hero.x == goal_pos_x && hero.y == goal_pos_y) {
            if (shielded_monsters < 4) {
                targets.sort(
                        [](const Entity &a, const Entity &b) { return a.health > b.health; });
                std::list<Entity>::iterator iterator;
                for (iterator = targets.begin(); iterator != targets.end(); iterator++) {
                    if (iterator->shield_life == 0 && (iterator->calculate_distance_to_target(hero.x, hero.y) < 2200)) {
                        cout << "SPELL SHIELD " << iterator->id << " " << hero_id << endl;
                        return;
                    }
                }
                send_count = 0;
            } else {
                //TODO: never reaches this code -> else condition doesn't happen if to low targets reach circle
                for (int i = 0; i < opp_heroes.size(); ++i) {
                    cerr << "want to control enemy hero with id: " << opp_heroes.at(i).id <<
                    " shield: " << opp_heroes.at(i).shield_life <<
                    " and in distance: " << opp_heroes.at(i).calculate_distance_to_target(hero.x, hero.y) << endl;

                    if ((opp_heroes.at(i).shield_life == 0) &&
                        (opp_heroes.at(i).calculate_distance_to_target(hero.x, hero.y) < 2200)) {
                        //TODO: doesn't work yet
                        cout << "SPELL CONTROL " << opp_heroes.at(i).id << " " << enemy_x << " " << enemy_y << " "
                             << hero_id << endl;
                        return;
                    }
                }
            }
            shielded_monsters = 0;
            cout << "SPELL WIND " << enemy_x << " " << enemy_y << " " << hero_id << endl;
        } else {
            cout << "MOVE " << goal_pos_x << " " << goal_pos_y << " " << hero_id << endl;
        }
    }

    void attack(Entity target) {
        int atk_x = target.x + target.vx;;
        int atk_y = target.y + target.vy;

        if (targets.size() > 1) {
            targets.pop_front();
            int distance_monsters = target.calculate_distance_to_target(targets.front().x,
                                                                        targets.front().y);
            if (distance_monsters < 800) { //rarely happens but slight improvement
                atk_x = (target.x + targets.front().x) / 2;
                atk_y = (target.y + targets.front().y) / 2;
            }
        }
        cout << "MOVE " << atk_x << " " << atk_y << " " << hero_id << endl;
    }

    void patrol() {
        if (forward) {
            if (hero.x == patrol_start_x && hero.y == patrol_start_y) {
                forward = false;
            }
        } else {
            if (hero.x == patrol_end_x && hero.y == patrol_end_y) {
                forward = true;
            }
        }

        if (forward) {
            cout << "MOVE " << patrol_start_x << " " << patrol_start_y << " " << hero_id << endl;
        } else {
            cout << "MOVE " << patrol_end_x << " " << patrol_end_y << " " << hero_id << endl;
        }
    }

    void defend_base_wind() {
        if (targets.front().distance_to_hero > 1200) { //monster is out of reach for wind
            defend_base_control();
        } else {
            cout << "SPELL WIND " << defend_base_x << " " << defend_base_y << " " << hero_id << endl;
        }
    }

    void defend_base_control() {
        if (targets.front().distance_to_hero > 2200) {
            cout << "MOVE " << targets.front().x << " " << targets.front().y << " " << hero_id << endl;
        } else {
            cout << "SPELL CONTROL " << targets.front().id << " " << defend_base_x << " " << defend_base_y << " "
                 << hero_id << endl;
        }

    }

    void prioritize_targets() {
        if (!targets.empty()) {
            if (hero_role == 1) {
                targets.sort(
                        [](const Entity &a, const Entity &b) { return a.distance_to_base < b.distance_to_base; });
            } else {
                targets.sort(
                        [](const Entity &a, const Entity &b) { return a.distance_to_hero < b.distance_to_hero; });
            }
        }
    }

    void is_hero_in_range_to_control(Entity opp_hero) {
        if (opp_hero.calculate_distance_to_target(hero.x, hero.y) < 2200) {
            enemy_hero_in_range_to_control = true;
        }
    }
};

int main() {
    int base_x; // The corner of the map representing your base
    int base_y;
    cin >> base_x >> base_y;
    cin.ignore();
    int heroes_per_player; // Always 3
    cin >> heroes_per_player;
    cin.ignore();
    int turn = 0;
    std::vector<Hero_States> hero_states;
    Hero_States hero_0;
    hero_0.calculate_positions(base_x, base_y, 6000, 800, 4500, 4500);
    hero_0.hero_role = 1;
    hero_0.hero_id = 0;
    hero_states.push_back(hero_0);
    Hero_States hero_1;
    hero_1.calculate_positions(base_x, base_y, 4500, 4500, 800, 6000);
    hero_1.hero_role = 1;
    hero_1.hero_id = 1;
    hero_states.push_back(hero_1);
    Hero_States hero_2;
    hero_2.calculate_positions(base_x, base_y, 5000, 7000, 10000, 2000);
    hero_2.hero_role = 2;
    hero_2.hero_id = 2;
    hero_states.push_back(hero_2);

    // game loop
    while (1) {
        int myHealth;
        int myMana;
        int oppHealth;
        int oppMana;
        int entity_count; // Amount of heroes and monsters you can see
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

        cerr << "finished input of turn: " << turn << endl;
        turn++;
        if (turn == 100) {
            hero_states.at(2).hero_role = 3;
        }
        cerr << "heroes array size is: " << heroes.size() << endl;
        cerr << "opp_heroes array size is: " << opp_heroes.size() << endl;
        cerr << "monsters array size is: " << monsters.size() << endl;

        for (int i = 0; i < heroes.size(); i++) {
            hero_states.at(i).hero = heroes.at(i);
        }

        for (int i = 0; i < hero_states.size(); i++) {
            hero_states.at(i).targets.clear();
            hero_states.at(i).enemy_hero_in_range_to_control = false;
            for (int j = 0; j < opp_heroes.size(); ++j) {
                hero_states.at(i).is_hero_in_range_to_control(opp_heroes.at(j));
            }
        }

        hero_states.at(2).opp_heroes = opp_heroes;

        for (int i = 0; i < monsters.size(); i++) {
            Entity monster = monsters.at(i);
            monster.distance_to_base = monster.calculate_distance_to_target(base_x, base_y);

            int distance_defender_one = monster.calculate_distance_to_target(hero_states.at(0).hero.x,
                                                                             hero_states.at(0).hero.y);
            int distance_defender_two = monster.calculate_distance_to_target(hero_states.at(1).hero.x,
                                                                             hero_states.at(1).hero.y);
            int distance_aggressor = monster.calculate_distance_to_target(hero_states.at(2).hero.x,
                                                                          hero_states.at(2).hero.y);

            if (monster.distance_to_base < 5000) {
                int min_hero_distance = std::min({distance_defender_one, distance_defender_two, distance_aggressor});
                int turns_until_catchup = (min_hero_distance - 800) / 400 + 1;
                int turns_until_base = monster.distance_to_base / 400;
                int hits_needed = monster.health / 2;
                if ((hits_needed + turns_until_catchup) > turns_until_base) {
                    cerr << "monster with id: " << monster.id << " has to much health(" << monster.health
                         << ") left, need two heroes" << endl;
                    monster.distance_to_hero = distance_defender_one;
                    hero_states.at(0).targets.push_back(monster);
                    Entity monster_copy = monster.copy();
                    monster_copy.distance_to_hero = distance_defender_two;
                    hero_states.at(1).targets.push_back(monster_copy);
                    continue;
                }
            }

            if (distance_defender_one < distance_defender_two) {
                if (distance_defender_one < distance_aggressor) {
                    monster.distance_to_hero = distance_defender_one;
                    hero_states.at(0).targets.push_back(monster);
                    continue;
                }
            } else {
                if (distance_defender_two < distance_aggressor) {
                    monster.distance_to_hero = distance_defender_two;
                    hero_states.at(1).targets.push_back(monster);
                    continue;
                }
            }
            monster.distance_to_hero = distance_aggressor;
            hero_states.at(2).targets.push_back(monster);
        }

        for (int i = 0; i < hero_states.size(); i++) {
            hero_states.at(i).prioritize_targets();
            hero_states.at(i).move();
        }
    }
}


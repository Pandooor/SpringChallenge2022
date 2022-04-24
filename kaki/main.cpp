#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <cmath>

using namespace std;

class Entity{
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

    int closestX;
    int closestY;
    int closestDistance = 99999;

    void calculate_distance_to_target(int base_x, int base_y){
        cerr << "calculate threat level for id" << id << endl;
        distance_to_base = sqrt(pow(base_x - x, 2) + pow(base_y - y, 2));
        cerr << "distance_to_base is: " << distance_to_base << endl;
    }

    void compare_distance_to_entity(list <Entity>::iterator it){
        int newClosestDistance = sqrt(pow(it->x - x, 2) + pow(it->y - y, 2));
        cerr << "check1 " << endl;
        if (newClosestDistance < closestDistance){
            closestDistance = newClosestDistance;
            closestX = it->x;
            closestY = it->y;
            cerr << "Updated closest distance to: " << closestDistance << endl;
        }
    }

};

int main()
{
    int base_x; // The corner of the map representing your base
    int base_y;
    cin >> base_x >> base_y; cin.ignore();
    int heroes_per_player; // Always 3
    cin >> heroes_per_player; cin.ignore();

    // game loop
    while (1) {
        int myHealth;
        int myMana;
        int oppHealth;
        int oppMana;
        int entity_count; // Amount of heros and monsters you can see
        cin >> myHealth >> myMana >> oppHealth >> oppMana >> entity_count; cin.ignore();

        std::list<Entity> heroes;
        std::list<Entity> opp_heroes;
        std::list<Entity> monsters;

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
            cin >> id >> type >> x >> y >> shield_life >> is_controlled >> health >> vx >> vy >> near_base >> threat_for; cin.ignore();

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

        cerr << "finished input" << endl;
        cerr << "heroes array size is: " << heroes.size() << endl;
        cerr << "opp_heroes array size is: " << opp_heroes.size() << endl;
        cerr << "monsters array size is: " << monsters.size() << endl;


        std::list<Entity>::iterator iterator;
        for (iterator = monsters.begin(); iterator != monsters.end(); ++iterator) {
            iterator->calculate_distance_to_target(base_x, base_y);
        }
        monsters.sort([](const Entity & a, const Entity & b) 
        { 
            if ( a.threat_for == 1 && b.threat_for != 1)
            {
                return true;
            }
            else if (a.threat_for != 1 && b.threat_for == 1)
            {
                return false;
            }
            else 
            {
                return a.distance_to_base < b.distance_to_base;
            }

        });

        for (iterator = monsters.begin(); iterator != monsters.end(); ++iterator) {
            cerr << " threat for : " << iterator-> threat_for << " Distance to base: " << iterator->distance_to_base << endl;
        }
        std::list<Entity>::iterator iterator_heroes;
        for (iterator_heroes = heroes.begin(); iterator_heroes != heroes.end();++iterator_heroes){
            for (iterator = monsters.begin(); iterator != monsters.end(); ++iterator) {
                if (iterator -> threat_for == 1){
                    iterator_heroes -> compare_distance_to_entity(iterator);
                }   
            }
        }
        cerr << "check1" << endl;

//        auto monsters_front = monsters.begin();
        for (int i = 0; i < heroes_per_player; i++) {
            Entity target;

            if (!monsters.empty()) {
                target = monsters.front();
                cerr << "target monsters id: " << target.id << " and distance_to_base: " << target.distance_to_base << "and threat" << target.threat_for << endl;
                cout << "MOVE " << target.x << " " << target.y << endl;
                monsters.pop_front();
            } else {
                if (i == 0) {cout << "MOVE " << "1500" << " " << "5500" << endl;}
                if (i == 1) {cout << "MOVE " << "4500" << " " << "4500" << endl;}
                if (i == 2) {cout << "MOVE " << "5500" << " " << "1500" << endl;}
                cerr << "MOVE for i: " << i << endl;
            }


            // Write an action using cout. DON'T FORGET THE "<< endl"
            // To debug: cerr << "Debug messages..." << endl;


            // In the first league: MOVE <x> <y> | WAIT; In later leagues: | SPELL <spellParams>;

        }

        //TODO upper defender
        //  patrol between x = 4500, y = 4500 and x = 6000, y = 800
        //  jump to the resque if something got inside the base circle


        //TODO lower defender
        //  patrol between x = 4500, y = 4500 and x = 800, y = 6000
        //  jump to the resque if something got inside the base circle


        //TODO aggressor


    }
}


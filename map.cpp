#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <random>

// Vector3 class for positions and physics
struct Vec3 {
    float x, y, z;
    Vec3(float X=0, float Y=0, float Z=0) : x(X), y(Y), z(Z) {}
    Vec3 operator+(const Vec3& o) const { return Vec3(x+o.x, y+o.y, z+o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x-o.x, y-o.y, z-o.z); }
    Vec3 operator*(float s) const { return Vec3(x*s, y*s, z*s); }
    float Length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 Normalized() const {
        float len = Length();
        if (len < 1e-6f) return Vec3(0,0,0);
        return (*this) * (1.0f / len);
    }
};

// Forward declarations
class Player;
class Weapon;

// Base class for all destructible objects
class DestructibleObject {
public:
    std::string name;
    Vec3 position;
    float health;
    bool isDestroyed = false;

    DestructibleObject(std::string n, Vec3 pos, float hp) : name(n), position(pos), health(hp) {}

    virtual void TakeDamage(float damage, const Weapon& weapon, Vec3 hitPoint) {
        if (isDestroyed) return;
        health -= damage;
        std::cout << name << " took " << damage << " damage. Health now: " << health << "\n";
        if (health <= 0) {
            isDestroyed = true;
            OnDestroyed(weapon, hitPoint);
        } else {
            OnDamaged(weapon, hitPoint);
        }
    }

    virtual void OnDamaged(const Weapon& weapon, Vec3 hitPoint) {
        // Default: no special effect
    }

    virtual void OnDestroyed(const Weapon& weapon, Vec3 hitPoint) {
        std::cout << name << " destroyed!\n";
    }

    virtual void Update(float deltaTime) {
        // For physics simulation if needed
    }

    virtual ~DestructibleObject() = default;
};

// Glass object that shatters on damage
class GlassObject : public DestructibleObject {
public:
    GlassObject(Vec3 pos) : DestructibleObject("Glass Pane", pos, 10.0f) {}

    void OnDamaged(const Weapon& weapon, Vec3 hitPoint) override {
        std::cout << "Glass cracked at (" << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z << ")\n";
    }

    void OnDestroyed(const Weapon& weapon, Vec3 hitPoint) override {
        std::cout << "Glass shattered at (" << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z << ")\n";
        SpawnGlassShards(hitPoint);
    }

    void SpawnGlassShards(Vec3 hitPoint) {
        std::cout << "Spawning glass shards with physics at " << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z << "\n";
        // Here you would spawn physics shards with random velocities
    }
};

// Crate object that explodes on destruction
class CrateObject : public DestructibleObject {
public:
    CrateObject(Vec3 pos) : DestructibleObject("Wooden Crate", pos, 50.0f) {}

    void OnDestroyed(const Weapon& weapon, Vec3 hitPoint) override {
        std::cout << "Crate exploded at (" << hitPoint.x << ", " << hitPoint.y << ", " << hitPoint.z << ")\n";
        Explode(hitPoint);
    }

    void Explode(Vec3 hitPoint) {
        std::cout << "Explosion force applied to nearby objects and players\n";
        // Apply explosion physics and damage to nearby objects and players
    }
};

// Building object with physics-based collapse
class BuildingObject : public DestructibleObject {
public:
    bool isCollapsing = false;
    float collapseProgress = 0.0f; // 0 to 1

    BuildingObject(std::string n, Vec3 pos, float hp) : DestructibleObject(n, pos, hp) {}

    void TakeDamage(float damage, const Weapon& weapon, Vec3 hitPoint) override {
        if (isDestroyed) return;

        health -= damage;
        std::cout << name << " took " << damage << " damage. Health now: " << health << "\n";

        if (health <= 0 && !isCollapsing) {
            isCollapsing = true;
            std::cout << name << " is collapsing!\n";
            StartCollapse();
        }
    }

    void StartCollapse() {
        // Start physics simulation of collapse
        // Break joints, spawn debris with physics impulses
        std::cout << "Building collapse physics started\n";
    }

    void Update(float deltaTime) override {
        if (isCollapsing) {
            collapseProgress += deltaTime * 0.2f; // collapse over 5 seconds
            if (collapseProgress >= 1.0f) {
                isDestroyed = true;
                std::cout << name << " has fully collapsed.\n";
                SpawnDebris();
            }
        }
    }

    void SpawnDebris() {
        std::cout << "Spawning debris with physics\n";
        // Spawn debris objects with physics forces
    }
};

// Weapon types
enum class WeaponType {
    Pistol,
    Rifle,
    Shotgun,
    GrenadeLauncher,
    HighImpact
};

class Weapon {
public:
    WeaponType type;
    float damage;
    float impactForce;

    Weapon(WeaponType t, float dmg, float force) : type(t), damage(dmg), impactForce(force) {}
};

// Player class with health and damage from debris
class Player {
public:
    std::string name;
    Vec3 position;
    float health = 100.0f;

    Player(std::string n, Vec3 pos) : name(n), position(pos) {}

    void TakeDamage(float damage) {
        health -= damage;
        std::cout << name << " took " << damage << " damage. Health now: " << health << "\n";
        if (health <= 0) {
            Die();
        }
    }

    void Die() {
        std::cout << name << " died!\n";
    }

    void OnDebrisHit(float debrisForce) {
        float damage = debrisForce * 0.5f; // damage proportional to force
        TakeDamage(damage);
    }
};

// Simple physics debris object
class Debris {
public:
    Vec3 position;
    Vec3 velocity;
    float mass;

    Debris(Vec3 pos, Vec3 vel, float m) : position(pos), velocity(vel), mass(m) {}

    void Update(float deltaTime) {
        // Simple physics integration
        position = position + velocity * deltaTime;
        velocity.y -= 9.81f * deltaTime; // gravity
    }

    bool CheckCollisionWithPlayer(const Player& player) {
        float dist = (position - player.position).Length();
        return dist < 1.0f; // collision radius
    }
};

// Map class holding all objects and players
class Map {
public:
    std::vector<std::shared_ptr<DestructibleObject>> destructibles;
    std::vector<Player> players;
    std::vector<Debris> debrisList;

    void Update(float deltaTime) {
        // Update destructibles
        for (auto& obj : destructibles) {
            obj->Update(deltaTime);
        }

        // Update debris physics and check collisions
        for (auto it = debrisList.begin(); it != debrisList.end();) {
            it->Update(deltaTime);

            bool debrisRemoved = false;
            for (auto& player : players) {
                if (it->CheckCollisionWithPlayer(player)) {
                    player.OnDebrisHit(it->mass * it->velocity.Length());
                    debrisRemoved = true;
                    break;
                }
            }

            if (debrisRemoved) {
                it = debrisList.erase(it);
            } else {
                ++it;
            }
        }
    }

    void ShootAtObject(int objIndex, const Weapon& weapon, Vec3 hitPoint) {
        if (objIndex < 0 || objIndex >= (int)destructibles.size()) return;
        destructibles[objIndex]->TakeDamage(weapon.damage, weapon, hitPoint);
    }
};

// -------------------- Demo --------------------

int main() {
    Map gameMap;

    // Add destructible objects
    gameMap.destructibles.push_back(std::make_shared<GlassObject>(Vec3(10, 2, 5)));
    gameMap.destructibles.push_back(std::make_shared<CrateObject>(Vec3(12, 0, 7)));
    gameMap.destructibles.push_back(std::make_shared<BuildingObject>("Building A", Vec3(15, 0, 10), 200.0f));

    // Add players
    gameMap.players.emplace_back("Player1", Vec3(11, 0, 6));
    gameMap.players.emplace_back("Player2", Vec3(16, 0, 11));

    // Weapons
    Weapon pistol(WeaponType::Pistol, 15.0f, 5.0f);
    Weapon grenadeLauncher(WeaponType::GrenadeLauncher, 100.0f, 50.0f);
    Weapon highImpactWeapon(WeaponType::HighImpact, 80.0f, 40.0f);

    // Simulate shooting glass with pistol
    std::cout << "\nPlayer shoots glass with pistol:\n";
    gameMap.ShootAtObject(0, pistol, Vec3(10, 2, 5));

    // Simulate shooting crate with grenade launcher
    std::cout << "\nPlayer shoots crate with grenade launcher:\n";
    gameMap.ShootAtObject(1, grenadeLauncher, Vec3(12, 0, 7));

    // Simulate shooting building with high impact weapon multiple times
    std::cout << "\nPlayer shoots building with high impact weapon:\n";
    for (int i = 0; i < 3; ++i) {
        gameMap.ShootAtObject(2, highImpactWeapon, Vec3(15, 0, 10));
        gameMap.Update(1.0f); // simulate 1 second per shot
    }

    // Simulate game update loop for debris and collapse
    for (int i = 0; i < 10; ++i) {
        gameMap.Update(0.5f);
    }

    return 0;
}

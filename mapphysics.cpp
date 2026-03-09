#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <algorithm>
#include <random>

/* =========================================================
   Basic Math
========================================================= */

struct Vec3
{
    float x=0,y=0,z=0;

    Vec3(){}
    Vec3(float X,float Y,float Z){x=X;y=Y;z=Z;}

    Vec3 operator+(const Vec3& r) const { return {x+r.x,y+r.y,z+r.z}; }
    Vec3 operator-(const Vec3& r) const { return {x-r.x,y-r.y,z-r.z}; }
    Vec3 operator*(float s) const { return {x*s,y*s,z*s}; }

    float Length() const { return std::sqrt(x*x+y*y+z*z); }
};

Vec3 Normalize(const Vec3& v)
{
    float l=v.Length();
    if(l<1e-6f) return {0,0,0};
    return {v.x/l,v.y/l,v.z/l};
}

/* =========================================================
   Weapon Types
========================================================= */

enum class WeaponType
{
    Pistol,
    Rifle,
    Shotgun,
    GrenadeLauncher,
    Explosion
};

/* =========================================================
   Forward Declarations
========================================================= */

class Player;

/* =========================================================
   Debris System
========================================================= */

struct DebrisObject
{
    float mass=1.0f;
    Vec3 position;
    Vec3 velocity;

    void Update(float dt)
    {
        velocity.y -= 9.81f * dt;
        position = position + velocity * dt;
    }

    float CalculateDamage() const
    {
        float impactForce = mass * velocity.Length();
        return impactForce * 0.05f;
    }
};

/* =========================================================
   Player
========================================================= */

class Player
{
public:

    float health = 100.f;
    Vec3 position{0,0,0};

    void TakeDamage(float damage)
    {
        health -= damage;

        std::cout<<"Player took "<<damage
                 <<" damage. Health="<<health<<"\n";

        if(health<=0)
            Die();
    }

    void Die()
    {
        std::cout<<"Player died.\n";
    }
};

/* =========================================================
   Base Destructible Object
========================================================= */

class DestructibleObject
{
public:

    float health = 100.f;

    virtual ~DestructibleObject() = default;

    virtual void TakeDamage(float damage,
                            WeaponType weapon,
                            const Vec3& hitPoint)=0;

    virtual void Update(float dt){}
};

/* =========================================================
   Glass Object
========================================================= */

class GlassObject : public DestructibleObject
{
public:

    float shatterThreshold = 15.f;
    bool shattered=false;

    std::vector<DebrisObject> shards;

    void TakeDamage(float damage,
                    WeaponType weapon,
                    const Vec3& hitPoint) override
    {
        if(shattered) return;

        if(damage>shatterThreshold)
        {
            Shatter(hitPoint);
            shattered=true;
        }
    }

    void Shatter(const Vec3& hitPoint)
    {
        std::cout<<"Glass shattered at "
                 <<hitPoint.x<<","<<hitPoint.y<<","<<hitPoint.z<<"\n";

        std::mt19937 rng(42);
        std::uniform_real_distribution<float> dist(-3,3);

        for(int i=0;i<10;i++)
        {
            DebrisObject shard;

            shard.mass=0.2f;
            shard.position=hitPoint;
            shard.velocity={dist(rng),dist(rng)+3,dist(rng)};

            shards.push_back(shard);
        }
    }

    void Update(float dt) override
    {
        for(auto& s:shards)
            s.Update(dt);
    }
};

/* =========================================================
   Exploding Crate
========================================================= */

class CrateObject : public DestructibleObject
{
public:

    float explosionThreshold = 40.f;
    bool exploded=false;

    std::vector<DebrisObject> debris;

    void TakeDamage(float damage,
                    WeaponType weapon,
                    const Vec3& hitPoint) override
    {
        if(exploded) return;

        if(damage>explosionThreshold || weapon==WeaponType::Explosion)
        {
            Explode(hitPoint);
            exploded=true;
        }
    }

    void Explode(const Vec3& hitPoint)
    {
        std::cout<<"Crate exploded at "
                 <<hitPoint.x<<","<<hitPoint.y<<","<<hitPoint.z<<"\n";

        std::mt19937 rng(7);
        std::uniform_real_distribution<float> dist(-6,6);

        for(int i=0;i<12;i++)
        {
            DebrisObject d;

            d.mass=1.0f;
            d.position=hitPoint;
            d.velocity={dist(rng),dist(rng)+5,dist(rng)};

            debris.push_back(d);
        }
    }

    void Update(float dt) override
    {
        for(auto& d:debris)
            d.Update(dt);
    }
};

/* =========================================================
   Building Object
========================================================= */

struct PhysicsPart
{
    Vec3 position;
    Vec3 velocity;
};

class BuildingObject : public DestructibleObject
{
public:

    float accumulatedDamage=0;
    float collapseThreshold=200;
    float highImpactThreshold=60;

    bool collapsed=false;

    std::vector<PhysicsPart> parts;

    BuildingObject()
    {
        for(int i=0;i<6;i++)
        {
            PhysicsPart p;
            p.position={float(i),5,0};
            parts.push_back(p);
        }
    }

    void TakeDamage(float damage,
                    WeaponType weapon,
                    const Vec3& hitPoint) override
    {
        if(collapsed) return;

        accumulatedDamage+=damage;

        if(weapon==WeaponType::GrenadeLauncher
        || damage>highImpactThreshold)
        {
            if(accumulatedDamage>collapseThreshold)
            {
                CollapseBuilding();
                collapsed=true;
            }
        }
    }

    void CollapseBuilding()
    {
        std::cout<<"Building collapsed!\n";

        std::mt19937 rng(99);
        std::uniform_real_distribution<float> dist(-4,4);

        for(auto& p:parts)
        {
            p.velocity={dist(rng),dist(rng)+6,dist(rng)};
        }
    }

    void Update(float dt) override
    {
        if(!collapsed) return;

        for(auto& p:parts)
        {
            p.velocity.y -= 9.81f*dt;
            p.position = p.position + p.velocity*dt;
        }
    }
};

/* =========================================================
   World Simulation
========================================================= */

class World
{
public:

    std::vector<std::unique_ptr<DestructibleObject>> objects;
    std::vector<Player> players;

    void Update(float dt)
    {
        for(auto& o:objects)
            o->Update(dt);
    }

    void DamageObject(int index,
                      float damage,
                      WeaponType weapon,
                      const Vec3& point)
    {
        if(index<0 || index>=objects.size()) return;

        objects[index]->TakeDamage(damage,weapon,point);
    }
};

/* =========================================================
   Demo
========================================================= */

int main()
{
    World world;

    world.players.push_back(Player());

    world.objects.push_back(
        std::make_unique<GlassObject>()
    );

    world.objects.push_back(
        std::make_unique<CrateObject>()
    );

    world.objects.push_back(
        std::make_unique<BuildingObject>()
    );

    Vec3 hit{1,2,3};

    std::cout<<"--- Rifle shot hits glass ---\n";
    world.DamageObject(0,20,WeaponType::Rifle,hit);

    std::cout<<"\n--- Grenade hits crate ---\n";
    world.DamageObject(1,60,WeaponType::Explosion,hit);

    std::cout<<"\n--- Launcher hits building ---\n";
    world.DamageObject(2,120,WeaponType::GrenadeLauncher,hit);

    float dt=1.0f/60.0f;

    for(int i=0;i<120;i++)
        world.Update(dt);

    std::cout<<"\nSimulation finished.\n";

    return 0;
}
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <optional>
#include <algorithm>
#include <random>

struct Vec3
{
    float x=0,y=0,z=0;

    Vec3(){}
    Vec3(float X,float Y,float Z){x=X;y=Y;z=Z;}

    Vec3 operator+(const Vec3& r)const{return {x+r.x,y+r.y,z+r.z};}
    Vec3 operator-(const Vec3& r)const{return {x-r.x,y-r.y,z-r.z};}
    Vec3 operator*(float s)const{return {x*s,y*s,z*s};}
    Vec3 operator/(float s)const{return {x/s,y/s,z/s};}
};

float Dot(const Vec3&a,const Vec3&b){return a.x*b.x+a.y*b.y+a.z*b.z;}
float LenSq(const Vec3&v){return Dot(v,v);}
float Len(const Vec3&v){return std::sqrt(LenSq(v));}

Vec3 Normalize(const Vec3&v)
{
    float l=Len(v);
    if(l<1e-6f) return {0,0,0};
    return v/l;
}

enum class MaterialType
{
    Flesh,
    Wood,
    Metal,
    Glass,
    Concrete,
    Explosive
};

enum class HitZone
{
    Head,
    Chest,
    Legs
};

struct DamageFalloff
{
    float nearDist=10;
    float farDist=60;
    float nearDmg=30;
    float farDmg=10;

    float Eval(float d)const
    {
        if(d<=nearDist) return nearDmg;
        if(d>=farDist) return farDmg;

        float t=(d-nearDist)/(farDist-nearDist);
        return nearDmg+(farDmg-nearDmg)*t;
    }
};

struct BulletProfile
{
    float mass=0.008f;
    float diameter=0.009f;
    float muzzleVelocity=360;
    float drag=0.15f;

    float penetrationPower=40;

    bool tracer=false;

    bool explosive=false;
    float explosionRadius=0;
    float explosionDamage=0;

    DamageFalloff falloff;
};

struct Projectile
{
    Vec3 pos;
    Vec3 vel;

    BulletProfile profile;

    float distance=0;
    float age=0;

    bool alive=true;
};

struct WorldObject
{
    std::string name;

    MaterialType material;

    Vec3 pos;
    float radius=1;

    float health=100;
    float armor=0;

    bool destructible=false;
};

struct HitResult
{
    bool hit=false;
    int objectIndex=-1;

    Vec3 point;
    Vec3 normal;
};

class World
{
public:

    std::vector<WorldObject> objects;
    std::vector<Projectile> bullets;

    Vec3 gravity={0,-9.81f,0};

    void SpawnBullet(const Vec3&pos,const Vec3&dir,const BulletProfile&p)
    {
        Projectile b;

        b.pos=pos;
        b.vel=dir*p.muzzleVelocity;
        b.profile=p;

        bullets.push_back(b);
    }

    void Tick(float dt)
    {
        for(auto &b:bullets)
        {
            if(!b.alive) continue;

            StepBullet(b,dt);
        }

        bullets.erase(
            std::remove_if(bullets.begin(),bullets.end(),
            [](const Projectile&p){return !p.alive;}),
            bullets.end()
        );
    }

private:

    void StepBullet(Projectile&b,float dt)
    {
        Vec3 prev=b.pos;

        Vec3 vdir=Normalize(b.vel);

        float speed=Len(b.vel);

        float dragForce=b.profile.drag*speed*speed;

        Vec3 drag=vdir*(-dragForce/b.profile.mass);

        Vec3 accel=gravity+drag;

        b.vel=b.vel+accel*dt;

        Vec3 next=b.pos+b.vel*dt;

        float segLen=Len(next-prev);

        b.distance+=segLen;
        b.age+=dt;

        if(b.profile.tracer)
        {
            std::cout<<"TRACER "<<b.pos.x<<" "<<b.pos.y<<" "<<b.pos.z<<"\n";
        }

        auto hit=Query(prev,next);

        if(hit.hit)
        {
            ResolveImpact(b,hit);
            b.pos=hit.point;
        }
        else
        {
            b.pos=next;
        }

        if(b.age>5) b.alive=false;
    }

    HitResult Query(const Vec3&a,const Vec3&b)
    {
        HitResult best;
        float bestT=1e9;

        for(int i=0;i<objects.size();i++)
        {
            auto&o=objects[i];

            Vec3 d=b-a;
            Vec3 m=a-o.pos;

            float A=Dot(d,d);
            float B=2*Dot(m,d);
            float C=Dot(m,m)-o.radius*o.radius;

            float disc=B*B-4*A*C;

            if(disc<0) continue;

            float t=(-B-std::sqrt(disc))/(2*A);

            if(t>=0 && t<=1 && t<bestT)
            {
                bestT=t;

                best.hit=true;
                best.objectIndex=i;

                best.point=a+d*t;
                best.normal=Normalize(best.point-o.pos);
            }
        }

        return best;
    }

    HitZone DetermineHitZone(const Vec3&point,const WorldObject&o)
    {
        float rel=point.y-o.pos.y;

        if(rel>0.5f) return HitZone::Head;
        if(rel<-0.3f) return HitZone::Legs;

        return HitZone::Chest;
    }

    void ResolveImpact(Projectile&b,const HitResult&hit)
    {
        auto&o=objects[hit.objectIndex];

        HitZone zone=DetermineHitZone(hit.point,o);

        float dmg=b.profile.falloff.Eval(b.distance);

        if(zone==HitZone::Head) dmg*=2;
        if(zone==HitZone::Legs) dmg*=0.7f;

        if(o.armor>0)
        {
            float pen=b.profile.penetrationPower/o.armor;

            if(pen<1)
            {
                std::cout<<"Armor stopped bullet\n";
                b.alive=false;
                return;
            }

            dmg*=0.7f;
        }

        if(o.material==MaterialType::Metal)
        {
            float angle=std::abs(Dot(Normalize(b.vel),hit.normal));

            if(angle<0.3f)
            {
                Vec3 r=b.vel-hit.normal*(2*Dot(b.vel,hit.normal));

                b.vel=r*0.5f;

                std::cout<<"Ricochet\n";
                return;
            }
        }

        if(o.material==MaterialType::Explosive)
        {
            SpawnExplosion(hit.point,5,120);
            b.alive=false;
            return;
        }

        if(o.destructible || o.material==MaterialType::Flesh)
        {
            o.health-=dmg;

            std::cout<<"Hit "<<o.name<<" damage "<<dmg<<" health "<<o.health<<"\n";

            if(o.health<=0)
            std::cout<<o.name<<" destroyed\n";
        }

        if(b.profile.explosive)
        {
            SpawnExplosion(hit.point,b.profile.explosionRadius,b.profile.explosionDamage);
        }

        b.alive=false;
    }

    void SpawnExplosion(const Vec3&p,float r,float dmg)
    {
        std::cout<<"Explosion at "<<p.x<<" "<<p.y<<" "<<p.z<<"\n";

        for(auto&o:objects)
        {
            float d=Len(o.pos-p);

            if(d>r) continue;

            float t=1-d/r;

            float final=dmg*t;

            o.health-=final;

            std::cout<<o.name<<" took "<<final<<" explosion damage\n";
        }
    }
};

BulletProfile Rifle()
{
    BulletProfile b;

    b.muzzleVelocity=900;
    b.penetrationPower=70;
    b.tracer=true;

    b.falloff.nearDist=20;
    b.falloff.farDist=200;
    b.falloff.nearDmg=35;
    b.falloff.farDmg=18;

    return b;
}

BulletProfile Pistol()
{
    BulletProfile b;

    b.muzzleVelocity=350;
    b.penetrationPower=30;

    b.falloff.nearDmg=28;
    b.falloff.farDmg=12;

    return b;
}

BulletProfile Grenade()
{
    BulletProfile b;

    b.muzzleVelocity=70;

    b.explosive=true;
    b.explosionRadius=6;
    b.explosionDamage=150;

    return b;
}

int main()
{
    World world;

    WorldObject dummy;
    dummy.name="Target Dummy";
    dummy.material=MaterialType::Flesh;
    dummy.pos={0,1,40};
    dummy.radius=0.6f;
    dummy.destructible=true;
    world.objects.push_back(dummy);

    WorldObject metal;
    metal.name="Metal Plate";
    metal.material=MaterialType::Metal;
    metal.pos={1,1,25};
    metal.radius=0.7f;
    metal.armor=80;
    world.objects.push_back(metal);

    WorldObject barrel;
    barrel.name="Fuel Barrel";
    barrel.material=MaterialType::Explosive;
    barrel.pos={-1,1,30};
    barrel.radius=0.6f;
    barrel.destructible=true;
    world.objects.push_back(barrel);

    Vec3 muzzle={0,1.5f,0};
    Vec3 dir=Normalize(Vec3{0,-0.02f,1});

    std::cout<<"FIRE RIFLE\n";
    world.SpawnBullet(muzzle,dir,Rifle());

    std::cout<<"FIRE PISTOL\n";
    world.SpawnBullet(muzzle,dir,Pistol());

    std::cout<<"FIRE GRENADE\n";
    world.SpawnBullet(muzzle,dir,Grenade());

    float dt=1.0f/120.0f;

    for(int i=0;i<1200;i++)
    {
        world.Tick(dt);
    }

    return 0;
}
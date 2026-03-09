#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>

// Vector3 for positions and directions
struct Vec3 {
    float x, y, z;
    Vec3(float X=0, float Y=0, float Z=0) : x(X), y(Y), z(Z) {}
};

// Abstract base class for particle effects
class ParticleEffect {
public:
    virtual void Play(const Vec3& position, const Vec3& direction) = 0;
    virtual ~ParticleEffect() = default;
};

// Concrete blood particle effect
class BloodParticleEffect : public ParticleEffect {
public:
    void Play(const Vec3& position, const Vec3& direction) override {
        std::cout << "[ParticleEffect] Blood splatter at (" << position.x << ", " << position.y << ", " << position.z << ")\n";
        // In real engine: spawn blood particle system oriented by direction
    }
};

// Abstract base class for animations
class Animation {
public:
    std::string name;
    float duration; // seconds

    Animation(std::string n, float d) : name(std::move(n)), duration(d) {}
    virtual void Play() {
        std::cout << "[Animation] Playing animation: " << name << "\n";
    }
    virtual ~Animation() = default;
};

// Animation manager to load and store animations
class AnimationManager {
public:
    void LoadAnimation(const std::string& name, float duration) {
        animations[name] = std::make_shared<Animation>(name, duration);
    }

    std::shared_ptr<Animation> GetAnimation(const std::string& name) {
        if (animations.find(name) != animations.end()) {
            return animations[name];
        }
        return nullptr;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<Animation>> animations;
};

// Weapon class with animation support
class Weapon {
public:
    std::string name;
    std::shared_ptr<Animation> fireAnimation;

    Weapon(std::string n) : name(std::move(n)) {}

    void SetFireAnimation(std::shared_ptr<Animation> anim) {
        fireAnimation = anim;
    }

    void Fire() {
        std::cout << name << " fired!\n";
        if (fireAnimation) {
            fireAnimation->Play();
        }
    }
};

// Player class with health, blood effect, and animations
class Player {
public:
    std::string name;
    float health = 100.0f;
    Vec3 position;

    std::shared_ptr<ParticleEffect> bloodEffect;
    AnimationManager animationManager;

    Player(std::string n, Vec3 pos) : name(std::move(n)), position(pos) {
        // Load animations
        animationManager.LoadAnimation("HitReaction", 1.0f);
        animationManager.LoadAnimation("Run", 2.0f);
        animationManager.LoadAnimation("Idle", 1.5f);

        // Create blood effect
        bloodEffect = std::make_shared<BloodParticleEffect>();
    }

    void TakeDamage(float damage, const Vec3& hitLocation, const Vec3& hitNormal) {
        health -= damage;
        std::cout << name << " took " << damage << " damage. Health now: " << health << "\n";

        // Spawn blood effect at hit location
        if (bloodEffect) {
            bloodEffect->Play(hitLocation, hitNormal);
        }

        // Play hit reaction animation
        auto hitAnim = animationManager.GetAnimation("HitReaction");
        if (hitAnim) {
            hitAnim->Play();
        }

        if (health <= 0) {
            Die();
        }
    }

    void Die() {
        std::cout << name << " died!\n";
        // Play death animation or ragdoll here
    }

    void PlayAnimation(const std::string& animName) {
        auto anim = animationManager.GetAnimation(animName);
        if (anim) {
            anim->Play();
        }
    }
};

// -------------------- Demo --------------------

int main() {
    // Create player
    Player player("Soldier1", Vec3(0, 0, 0));

    // Create weapon and assign fire animation
    Weapon rifle("Assault Rifle");
    AnimationManager weaponAnimMgr;
    weaponAnimMgr.LoadAnimation("RifleFire", 0.5f);
    rifle.SetFireAnimation(weaponAnimMgr.GetAnimation("RifleFire"));

    // Simulate firing weapon
    rifle.Fire();

    // Simulate player taking damage with blood effect and animation
    Vec3 hitLocation(1.0f, 1.5f, 0.0f);
    Vec3 hitNormal(0.0f, 1.0f, 0.0f);
    player.TakeDamage(25.0f, hitLocation, hitNormal);

    // Simulate player running animation
    player.PlayAnimation("Run");

    return 0;
}

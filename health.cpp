#include <iostream>
#include <iomanip>

class HealthSystem {
public:
    HealthSystem(int maxHealth = 100, float healDelay = 5.0f, float healRate = 1.0f)
        : maxHealth(maxHealth), currentHealth(maxHealth),
          healDelay(healDelay), healRate(healRate),
          timeSinceLastDamage(healDelay) // start ready to heal
    {}

    // Call this when player takes damage
    void TakeDamage(int damage) {
        if (damage <= 0) return;

        currentHealth -= damage;
        if (currentHealth < 0) currentHealth = 0;

        timeSinceLastDamage = 0.0f; // reset heal timer
        std::cout << "[DAMAGE] Took " << damage << " damage. Health now: " << currentHealth << "\n";
    }

    // Call every frame with delta time in seconds
    void Update(float deltaTime) {
        if (currentHealth >= maxHealth) {
            currentHealth = maxHealth;
            return; // no healing needed
        }

        timeSinceLastDamage += deltaTime;

        if (timeSinceLastDamage >= healDelay) {
            // Heal over time
            float healAmount = healRate * deltaTime;
            currentHealth += healAmount;

            if (currentHealth > maxHealth) currentHealth = maxHealth;

            std::cout << "[HEAL] Healing... Health now: " << std::fixed << std::setprecision(2) << currentHealth << "\n";
        }
    }

    int GetHealth() const { return static_cast<int>(currentHealth); }

private:
    int maxHealth;
    float currentHealth;
    float healDelay;          // seconds to wait before healing starts
    float healRate;           // health per second
    float timeSinceLastDamage; // time since last damage taken
};

// -------------------- Demo --------------------

int main() {
    HealthSystem playerHealth;

    float simulationTime = 20.0f; // seconds
    float dt = 0.5f;              // simulate update every 0.5 seconds

    // Simulate damage at 2 seconds and 8 seconds
    float damageTimes[] = {2.0f, 8.0f};
    int damageIndex = 0;

    for (float t = 0.0f; t <= simulationTime; t += dt) {
        std::cout << "Time: " << t << "s\n";

        if (damageIndex < 2 && t >= damageTimes[damageIndex]) {
            playerHealth.TakeDamage(20);
            damageIndex++;
        }

        playerHealth.Update(dt);
    }

    return 0;
}

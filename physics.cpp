#include <iostream>
#include <vector>
#include <cmath>

// Vector3 class for 3D physics calculations
struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vector3 operator+(const Vector3& rhs) const {
        return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    Vector3 operator-(const Vector3& rhs) const {
        return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3 operator/(float scalar) const {
        return Vector3(x / scalar, y / scalar, z / scalar);
    }

    Vector3& operator+=(const Vector3& rhs) {
        x += rhs.x; y += rhs.y; z += rhs.z;
        return *this;
    }

    float Length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    Vector3 Normalized() const {
        float len = Length();
        if (len == 0) return Vector3(0,0,0);
        return *this / len;
    }

    float Dot(const Vector3& rhs) const {
        return x*rhs.x + y*rhs.y + z*rhs.z;
    }

    Vector3 Cross(const Vector3& rhs) const {
        return Vector3(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
    }
};

// Physics object representing a rigid body
class PhysicsObject {
public:
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;
    float mass;
    float radius; // For simple sphere collision
    bool isStatic; // If true, object does not move

    PhysicsObject(Vector3 pos, float m, float r, bool stat = false)
        : position(pos), mass(m), radius(r), isStatic(stat) {
        velocity = Vector3(0,0,0);
        acceleration = Vector3(0,0,0);
    }

    // Apply a force to the object (F = ma)
    void ApplyForce(const Vector3& force) {
        if (isStatic) return;
        Vector3 a = force / mass;
        acceleration += a;
    }

    // Update physics state with time step dt
    void Update(float dt) {
        if (isStatic) return;

        // Integrate acceleration to velocity
        velocity += acceleration * dt;

        // Integrate velocity to position
        position += velocity * dt;

        // Reset acceleration for next frame
        acceleration = Vector3(0,0,0);
    }
};

// Simple physics world to manage objects and simulate physics
class PhysicsWorld {
public:
    std::vector<PhysicsObject*> objects;
    Vector3 gravity = Vector3(0, -9.81f, 0); // Gravity force

    // Friction coefficient (simple ground friction)
    float frictionCoefficient = 0.5f;

    // Air drag coefficient
    float dragCoefficient = 0.1f;

    // Ground plane height (y = 0)
    float groundHeight = 0.0f;

    void AddObject(PhysicsObject* obj) {
        objects.push_back(obj);
    }

    void Simulate(float dt) {
        for (auto obj : objects) {
            if (obj->isStatic) continue;

            // Apply gravity force: F = m * g
            obj->ApplyForce(gravity * obj->mass);

            // Apply drag force: F_drag = -dragCoefficient * v
            Vector3 dragForce = obj->velocity * -dragCoefficient;
            obj->ApplyForce(dragForce);

            // Update object physics
            obj->Update(dt);

            // Collision with ground plane
            HandleGroundCollision(obj);
        }
    }

private:
    void HandleGroundCollision(PhysicsObject* obj) {
        // Check if object is below ground (considering radius)
        if (obj->position.y - obj->radius < groundHeight) {
            // Correct position to ground level
            obj->position.y = groundHeight + obj->radius;

            // Reflect velocity on Y axis (simple elastic collision)
            if (obj->velocity.y < 0) {
                obj->velocity.y = -obj->velocity.y * 0.7f; // 0.7 = restitution coefficient (some energy loss)
            }

            // Apply friction to horizontal velocity (x and z)
            obj->velocity.x *= (1.0f - frictionCoefficient);
            obj->velocity.z *= (1.0f - frictionCoefficient);

            // If velocity is very small, stop the object to simulate rest
            if (std::abs(obj->velocity.y) < 0.1f) {
                obj->velocity.y = 0;
            }
            if (std::abs(obj->velocity.x) < 0.01f) {
                obj->velocity.x = 0;
            }
            if (std::abs(obj->velocity.z) < 0.01f) {
                obj->velocity.z = 0;
            }
        }
    }
};

int main() {
    PhysicsWorld world;

    // Create a dynamic sphere object above the ground
    PhysicsObject* ball = new PhysicsObject(Vector3(0, 10, 0), 1.0f, 0.5f);
    world.AddObject(ball);

    // Create a static ground object (not moving, just for reference)
    PhysicsObject* ground = new PhysicsObject(Vector3(0, 0, 0), 0.0f, 0.0f, true);
    world.AddObject(ground);

    float simulationTime = 5.0f; // seconds
    float dt = 0.01f; // time step (100 FPS)

    for (float t = 0; t < simulationTime; t += dt) {
        world.Simulate(dt);

        // Output ball position and velocity
        std::cout << "Time: " << t
                  << " | Position: (" << ball->position.x << ", " << ball->position.y << ", " << ball->position.z << ")"
                  << " | Velocity: (" << ball->velocity.x << ", " << ball->velocity.y << ", " << ball->velocity.z << ")"
                  << std::endl;
    }

    // Clean up
    delete ball;
    delete ground;

    return 0;
}

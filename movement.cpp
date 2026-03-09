#include <iostream>
#include <cmath>

// Enum for movement states
enum class MovementState {
    Walking,
    Sprinting,
    Crouching,
    Sliding,
    Idle
};

class Character {
public:
    // Movement speeds (units per second)
    float walkSpeed = 200.0f;
    float sprintSpeed = 400.0f;
    float crouchSpeed = 100.0f;
    float slideSpeed = 500.0f;

    // Current velocity vector (x, y)
    float velocityX = 0.0f;
    float velocityY = 0.0f;

    // Current movement state
    MovementState currentState = MovementState::Idle;

    // Flags for input
    bool inputForward = false;
    bool inputBackward = false;
    bool inputLeft = false;
    bool inputRight = false;
    bool inputSprint = false;
    bool inputCrouch = false;
    bool inputSlide = false;

    // Position (for demonstration)
    float posX = 0.0f;
    float posY = 0.0f;

    // Sliding timer and duration
    float slideDuration = 1.0f; // seconds
    float slideTimer = 0.0f;
    bool isSliding = false;

    // Update function called every frame with deltaTime in seconds
    void Update(float deltaTime) {
        HandleStateTransitions();
        CalculateVelocity();
        ApplyMovement(deltaTime);
        UpdateSlideTimer(deltaTime);
        PrintStatus();
    }

    // Simulate input setting (in real scenario, input comes from keyboard/controller)
    void SetInput(bool forward, bool backward, bool left, bool right, bool sprint, bool crouch, bool slide) {
        inputForward = forward;
        inputBackward = backward;
        inputLeft = left;
        inputRight = right;
        inputSprint = sprint;
        inputCrouch = crouch;
        inputSlide = slide;
    }

private:
    void HandleStateTransitions() {
        // Sliding has highest priority if active
        if (isSliding) {
            currentState = MovementState::Sliding;
            return;
        }

        // Start sliding if slide input pressed while sprinting and moving forward
        if (inputSlide && currentState == MovementState::Sprinting && inputForward) {
            StartSlide();
            return;
        }

        // Crouching overrides walking/sprinting
        if (inputCrouch) {
            currentState = MovementState::Crouching;
            return;
        }

        // Sprinting if sprint input and moving forward
        if (inputSprint && inputForward) {
            currentState = MovementState::Sprinting;
            return;
        }

        // Walking if moving in any direction
        if (inputForward || inputBackward || inputLeft || inputRight) {
            currentState = MovementState::Walking;
            return;
        }

        // Otherwise idle
        currentState = MovementState::Idle;
    }

    void CalculateVelocity() {
        float speed = 0.0f;

        switch (currentState) {
            case MovementState::Walking:
                speed = walkSpeed;
                break;
            case MovementState::Sprinting:
                speed = sprintSpeed;
                break;
            case MovementState::Crouching:
                speed = crouchSpeed;
                break;
            case MovementState::Sliding:
                speed = slideSpeed;
                break;
            case MovementState::Idle:
                speed = 0.0f;
                break;
        }

        // Calculate direction vector based on input
        float dirX = 0.0f;
        float dirY = 0.0f;

        if (inputForward) dirY += 1.0f;
        if (inputBackward) dirY -= 1.0f;
        if (inputRight) dirX += 1.0f;
        if (inputLeft) dirX -= 1.0f;

        // Normalize direction vector to prevent faster diagonal movement
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        if (length > 0) {
            dirX /= length;
            dirY /= length;
        }

        // Set velocity
        velocityX = dirX * speed;
        velocityY = dirY * speed;
    }

    void ApplyMovement(float deltaTime) {
        posX += velocityX * deltaTime;
        posY += velocityY * deltaTime;
    }

    void StartSlide() {
        isSliding = true;
        slideTimer = slideDuration;
        currentState = MovementState::Sliding;
        std::cout << "Slide started!" << std::endl;
    }

    void UpdateSlideTimer(float deltaTime) {
        if (isSliding) {
            slideTimer -= deltaTime;
            if (slideTimer <= 0.0f) {
                isSliding = false;
                std::cout << "Slide ended." << std::endl;
            }
        }
    }

    void PrintStatus() {
        std::string stateStr;
        switch (currentState) {
            case MovementState::Walking: stateStr = "Walking"; break;
            case MovementState::Sprinting: stateStr = "Sprinting"; break;
            case MovementState::Crouching: stateStr = "Crouching"; break;
            case MovementState::Sliding: stateStr = "Sliding"; break;
            case MovementState::Idle: stateStr = "Idle"; break;
        }

        std::cout << "State: " << stateStr
                  << " | Position: (" << posX << ", " << posY << ")"
                  << " | Velocity: (" << velocityX << ", " << velocityY << ")"
                  << std::endl;
    }
};

int main() {
    Character player;

    // Simulate a game loop with fixed deltaTime (e.g., 0.1 seconds per frame)
    float deltaTime = 0.1f;

    // Example input sequence:
    // 1. Walk forward for 1 second
    for (int i = 0; i < 10; ++i) {
        player.SetInput(true, false, false, false, false, false, false);
        player.Update(deltaTime);
    }

    // 2. Sprint forward for 1 second
    for (int i = 0; i < 10; ++i) {
        player.SetInput(true, false, false, false, true, false, false);
        player.Update(deltaTime);
    }

    // 3. Slide while sprinting forward for 1.5 seconds
    for (int i = 0; i < 15; ++i) {
        player.SetInput(true, false, false, false, true, false, true);
        player.Update(deltaTime);
    }

    // 4. Crouch and walk left for 1 second
    for (int i = 0; i < 10; ++i) {
        player.SetInput(false, false, true, false, false, true, false);
        player.Update(deltaTime);
    }

    // 5. Idle for 1 second
    for (int i = 0; i < 10; ++i) {
        player.SetInput(false, false, false, false, false, false, false);
        player.Update(deltaTime);
    }

    return 0;
}

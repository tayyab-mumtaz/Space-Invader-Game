#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>

using namespace sf;
using namespace std;

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const int PLAYER_SPEED = 16;
const int BULLET_SPEED = 15;
const float ALIEN_SPAWN_INTERVAL = 2.0f;
const int MAX_HEARTS = 3;
const float HEART_SPAWN_INTERVAL = 7.0f;
bool soundEnabled = true;

//Levels and speed constants
enum Level { EASY, MEDIUM, HARD };

//Structures
struct BonusHeart {
    Sprite shape;
    bool active = true;
};

struct Bullet {
    RectangleShape shape;
    bool active = true;
};

struct Alien {
    Sprite alien;
    bool active = true;
};

//Functions
Level displayDifficultyPage(RenderWindow& window, Font& font);
void displayHomePage(RenderWindow& window, Font& font);
void displayOptionsMenu(RenderWindow& window, Font& font, bool& soundEnabled);
void readHighScores(int highScores[]);
void writeHighScores(const int highScores[]);

//File to store high scores
const string HIGH_SCORE_FILE = "texture/highscores.txt";

//Main
int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "RETRO BLASTERS", Style::Fullscreen);
    window.setFramerateLimit(60);

    Font font;
    if (!font.loadFromFile("texture/font3.ttf")) {
        cerr << "Error: Could not load font!" << endl;
        return -1;
    }

    int score = 0;
    int highScores[3] = { 0, 0, 0 };
    readHighScores(highScores);


    // Main game loop
    bool playAgain = true;
    while (playAgain && window.isOpen()) {
        displayHomePage(window, font);
        Level currentLevel = displayDifficultyPage(window, font);
        score = 0;

        int currentLevelIndex = 0;
        int alienSpeed;
        switch (currentLevel) {
        case Level::EASY:
            currentLevelIndex = 0;
            alienSpeed = 6.0f;
            break;
        case Level::MEDIUM:
            currentLevelIndex = 1;
            alienSpeed = 8.0f;
            break;
        case Level::HARD:
            currentLevelIndex = 2;
            alienSpeed = 10.0f;
            break;
        }

        // Load background texture
        Texture backgroundTexture;
        if (!backgroundTexture.loadFromFile("texture/back ground.jpg")) {
            cerr << "Error: Could not load background texture!" << endl;
            return -1;
        }
        Sprite background(backgroundTexture);
        background.setScale(
            static_cast<float>(WINDOW_WIDTH) / background.getLocalBounds().width,
            static_cast<float>(WINDOW_HEIGHT) / background.getLocalBounds().height
        );

        // Player setup
        Texture playerTexture;
        if (!playerTexture.loadFromFile("texture/sprite.png")) {
            cerr << "Error: Could not load player texture!" << endl;
            return -1;
        }
        Sprite player(playerTexture);
        player.setScale(0.1f, 0.1f);
        player.setPosition(WINDOW_WIDTH / 2 - player.getGlobalBounds().width / 2, WINDOW_HEIGHT - player.getGlobalBounds().height - 10);

        // Bullets
        vector<Bullet> bullets;
        Clock shootingClock;

        // Aliens
        vector<Alien> aliens;
        Texture alienTexture;
        if (!alienTexture.loadFromFile("texture/alien.png")) {
            cerr << "Error: Could not load alien texture!" << endl;
            return -1;
        }
        Sprite alienSprite(alienTexture);
        alienSprite.setScale(0.1f, 0.1f);
        Clock spawnClock;

        // Hearts
        int hearts = MAX_HEARTS;
        Texture heartTexture;
        if (!heartTexture.loadFromFile("texture/heart1.png")) {
            cerr << "Error: Could not load heart texture!" << endl;
            return -1;
        }
        Sprite heartSprite(heartTexture);
        heartSprite.setScale(0.04f, 0.04f);

        // Randomly generated hearts
        vector<BonusHeart> bonusHearts;
        Clock heartSpawnClock;

        // Game Over image
        Texture gameOverTexture;
        if (!gameOverTexture.loadFromFile("texture/over.png")) {
            cerr << "Error: Could not load Game Over image!" << endl;
            return -1;
        }
        Sprite gameOverSprite(gameOverTexture);
        gameOverSprite.setScale(1.5, 1.5);

        // Declare sound buffers and sounds
        SoundBuffer shootBuffer, alienDestroyedBuffer, gameOverBuffer, heartCollectedBuffer, navigationBuffer, selectionBuffer;
        Sound shootSound, alienDestroyedSound, gameOverSound, heartCollectedSound, navigationSound, selectionSound;

        // Load sound files
        if (!shootBuffer.loadFromFile("texture/bullets.mp3") ||
            !gameOverBuffer.loadFromFile("texture/gameover.mp3") ||
            !heartCollectedBuffer.loadFromFile("texture/hrt pick.mp3")) {
            cerr << "Error loading sound files!" << endl;
            return -1;
        }

        // Assign buffers to sounds
        shootSound.setBuffer(shootBuffer);
        gameOverSound.setBuffer(gameOverBuffer);
        heartCollectedSound.setBuffer(heartCollectedBuffer);

        Music backgroundMusic;
        if (!backgroundMusic.openFromFile("texture/background sound.mp3")) {
            cerr << "Error loading background music!" << endl;
            return -1;
        }
        backgroundMusic.setLoop(true);
        if (soundEnabled) {
            backgroundMusic.play();
        }

        // Game loop
        score = 0;
        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();
            }
            if (Keyboard::isKeyPressed(Keyboard::Escape)) {
                window.close();
            }
            // Player movement
            if (Keyboard::isKeyPressed(Keyboard::Left) && player.getPosition().x > 0) {
                player.move(-PLAYER_SPEED, 0);
            }
            if (Keyboard::isKeyPressed(Keyboard::Right) && player.getPosition().x + player.getGlobalBounds().width < WINDOW_WIDTH) {
                player.move(PLAYER_SPEED, 0);
            }
            if (Keyboard::isKeyPressed(Keyboard::Up) && player.getPosition().y > 0) {
                player.move(0, -PLAYER_SPEED);
            }
            if (Keyboard::isKeyPressed(Keyboard::Down) && player.getPosition().y + player.getGlobalBounds().height < WINDOW_HEIGHT) {
                player.move(0, PLAYER_SPEED);
            }

            // Spawn aliens
            if (spawnClock.getElapsedTime().asSeconds() >= ALIEN_SPAWN_INTERVAL) {
                Alien alien;
                alien.alien.setTexture(alienTexture);
                alien.alien.setScale(0.1f, 0.1f);
                float xPosition = static_cast<float>(rand() % (WINDOW_WIDTH - static_cast<int>(alien.alien.getGlobalBounds().width)));
                alien.alien.setPosition(xPosition, -alien.alien.getGlobalBounds().width);
                aliens.push_back(alien);
                spawnClock.restart();
            }

            //Spawn hearts
            if (heartSpawnClock.getElapsedTime().asSeconds() >= HEART_SPAWN_INTERVAL) {
                BonusHeart newHeart;
                newHeart.shape.setTexture(heartTexture);
                newHeart.shape.setScale(0.04f, 0.04f);
                float xPosition = static_cast<float>(rand() % (WINDOW_WIDTH - static_cast<int>(newHeart.shape.getGlobalBounds().width)));
                newHeart.shape.setPosition(xPosition, -newHeart.shape.getGlobalBounds().height);
                bonusHearts.push_back(newHeart);
                heartSpawnClock.restart();
            }

            // Shooting bullets
            if (Keyboard::isKeyPressed(Keyboard::Space) && bullets.size() < 5) {
                if (shootingClock.getElapsedTime().asSeconds() >= 0.2f) {
                    if (soundEnabled) {
                        shootSound.play();
                    }
                    Bullet bullet;
                    bullet.shape.setSize(Vector2f(2, 30));
                    bullet.shape.setFillColor(Color::Green);
                    bullet.shape.setPosition(player.getPosition().x + player.getGlobalBounds().width / 2 - 2.5f, player.getPosition().y);
                    bullets.push_back(bullet);
                    shootingClock.restart();
                }
            }

            // Move bullets
            for (auto& bullet : bullets) {
                if (bullet.active) {
                    bullet.shape.move(0, -BULLET_SPEED);
                    if (bullet.shape.getPosition().y < 0) {
                        bullet.active = false;
                    }
                }
            }
            bullets.erase(remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return !b.active; }), bullets.end());

            // Move aliens
            for (auto& alien : aliens) {
                if (alien.active) {
                    alien.alien.move(0, alienSpeed);
                    if (alien.alien.getPosition().x + alien.alien.getGlobalBounds().width < 0 || alien.alien.getPosition().y > WINDOW_HEIGHT) {
                        alien.active = false;
                        hearts--;
                    }
                }
            }

            // Remove inactive aliens
            aliens.erase(remove_if(aliens.begin(), aliens.end(), [](const Alien& a) { return !a.active; }), aliens.end());

            //Move hearts
            for (auto& heart : bonusHearts) {
                if (heart.active) {
                    heart.shape.move(0, alienSpeed);
                    if (heart.shape.getPosition().y > WINDOW_HEIGHT) {
                        heart.active = false;
                    }
                }
            }
            bonusHearts.erase(remove_if(bonusHearts.begin(), bonusHearts.end(), [](const BonusHeart& h) { return !h.active; }), bonusHearts.end());

            // Check collisions
            for (auto& bullet : bullets) {
                for (auto& alien : aliens) {
                    if (bullet.active && alien.active && bullet.shape.getGlobalBounds().intersects(alien.alien.getGlobalBounds())) {
                        bullet.active = false;
                        alien.active = false;
                        score++;
                    }
                }
            }

            // Collision with player (alien collides with spaceship)
            for (auto& alien : aliens) {
                if (alien.active && player.getGlobalBounds().intersects(alien.alien.getGlobalBounds())) {
                    alien.active = false;
                    hearts--;
                }
            }

            //Collision of hearts with player
            for (auto& heart : bonusHearts) {
                if (heart.active && player.getGlobalBounds().intersects(heart.shape.getGlobalBounds())) {
                    heart.active = false;
                    if (hearts < MAX_HEARTS) {
                        hearts++;

                        if (soundEnabled) {
                            heartCollectedSound.play();
                        }
                    }
                }
            }

            // Render
            window.clear();
            window.draw(background);
            window.draw(player);

            for (const auto& bullet : bullets) {
                window.draw(bullet.shape);
            }

            for (const auto& alien : aliens) {
                window.draw(alien.alien);
            }

            // Display hearts
            for (int i = 0; i < hearts; i++) {
                heartSprite.setPosition(10 + (i * (heartSprite.getGlobalBounds().width + 5)), 10);
                window.draw(heartSprite);
            }

            //Display score and high score
            Text scoreText;
            scoreText.setFont(font);
            scoreText.setCharacterSize(60);
            scoreText.setFillColor(Color::White);
            scoreText.setString("Score: " + to_string(score));
            scoreText.setPosition(800, 10);
            window.draw(scoreText);

            Text highScoreText;
            highScoreText.setFont(font);
            highScoreText.setCharacterSize(60);
            highScoreText.setFillColor(Color::White);
            highScoreText.setString("Highest Score: " + to_string(highScores[currentLevelIndex]));
            highScoreText.setPosition(1300, 10);
            window.draw(highScoreText);

            //Display spawning hearts
            for (const auto& heart : bonusHearts) {
                if (heart.active) {
                    window.draw(heart.shape);
                }
            }

            window.display();

            // Update the high score for the current level
            if (hearts <= 0) {
                if (score > highScores[currentLevelIndex]) {
                    highScores[currentLevelIndex] = score;
                    writeHighScores(highScores);
                }
                scoreText.setString("Your Score: " + to_string(score));
                scoreText.setPosition(100, 10);

                if (soundEnabled) {
                    gameOverSound.play();
                    backgroundMusic.stop();
                }
                window.clear();
                gameOverSprite.setPosition(700, 350);
                window.draw(gameOverSprite);
                window.draw(scoreText);
                window.draw(highScoreText);
                window.display();

                bool waitingForInput = true;
                while (waitingForInput) {
                    Event event;
                    while (window.pollEvent(event)) {
                        if (event.type == Event::Closed || (event.type == Event::KeyPressed)) {
                            waitingForInput = false;
                            playAgain = true;

                            currentLevel = displayDifficultyPage(window, font);
                            currentLevelIndex = (currentLevel == Level::EASY ? 0 : (currentLevel == Level::MEDIUM ? 1 : 2));

                            switch (currentLevel) {
                            case Level::EASY:
                                alienSpeed = 6.0f;
                                break;
                            case Level::MEDIUM:
                                alienSpeed = 8.0f;
                                break;
                            case Level::HARD:
                                alienSpeed = 10.0f;
                                break;
                            }

                            // Reset the game variables (e.g., hearts, player position, etc.)
                            hearts = MAX_HEARTS;
                            score = 0;
                            aliens.clear();
                            bullets.clear();
                            player.setPosition(WINDOW_WIDTH / 2 - player.getGlobalBounds().width / 2, WINDOW_HEIGHT - player.getGlobalBounds().height - 10);
                            spawnClock.restart();
                            if (soundEnabled) {
                                backgroundMusic.play();
                            }
                        }
                    }
                }
            }

        }
    }
    return 0;
}

// Function to display the difficulty level selection page
Level displayDifficultyPage(RenderWindow& window, Font& font) {
    Texture difficultyTexture;
    if (!difficultyTexture.loadFromFile("texture/main page.jpg")) {
        cerr << "Error: Could not load difficulty background texture!" << endl;
    }
    Sprite difficultyPage(difficultyTexture);
    difficultyPage.setScale(
        static_cast<float>(WINDOW_WIDTH) / difficultyPage.getLocalBounds().width,
        static_cast<float>(WINDOW_HEIGHT) / difficultyPage.getLocalBounds().height
    );

    Text easyText("EASY", font, 90);
    easyText.setPosition(880, 700);
    easyText.setFillColor(Color::Red);

    Text mediumText("MEDIUM", font, 90);
    mediumText.setPosition(830, 800);
    mediumText.setFillColor(Color::White);

    Text hardText("HARD", font, 90);
    hardText.setPosition(880, 900);
    hardText.setFillColor(Color::White);

    Text endText("Click BackSpace to return to main page.", font, 40);
    endText.setPosition(585, 1000);
    endText.setFillColor(Color::Black);

    SoundBuffer navigationBuffer, selectionBuffer;
    Sound navigationSound, selectionSound;

    if (!navigationBuffer.loadFromFile("texture/navigation.mp3") ||
        !selectionBuffer.loadFromFile("texture/selection.mp3")) {
        cerr << "Error loading sound files for navigation or selection!" << endl;
    }

    navigationSound.setBuffer(navigationBuffer);
    selectionSound.setBuffer(selectionBuffer);

    Level selectedLevel = Level::EASY;
    bool selecting = true;

    while (selecting) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Escape) {
                    window.close();
                }
                if (event.key.code == Keyboard::BackSpace) {
                    displayHomePage(window, font);
                }

                if (event.key.code == Keyboard::Up) {
                    if (selectedLevel == Level::MEDIUM) {
                        selectedLevel = Level::EASY;
                        navigationSound.play();
                    }
                    else if (selectedLevel == Level::HARD) {
                        selectedLevel = Level::MEDIUM;
                        navigationSound.play();
                    }
                }

                if (event.key.code == Keyboard::Down) {
                    if (selectedLevel == Level::EASY) {
                        selectedLevel = Level::MEDIUM;
                        navigationSound.play();
                    }
                    else if (selectedLevel == Level::MEDIUM) {
                        selectedLevel = Level::HARD;
                        navigationSound.play();
                    }
                }

                if (event.key.code == Keyboard::Enter) {
                    selecting = false;
                }
            }
        }

        // Render difficulty page
        window.clear();
        window.draw(difficultyPage);
        window.draw(easyText);
        window.draw(mediumText);
        window.draw(hardText);
        window.draw(endText);

        //Difficulty
        if (selectedLevel == Level::EASY)
            easyText.setFillColor(Color::Red);
        else
            easyText.setFillColor(Color::White);

        if (selectedLevel == Level::MEDIUM)
            mediumText.setFillColor(Color::Red);
        else
            mediumText.setFillColor(Color::White);

        if (selectedLevel == Level::HARD)
            hardText.setFillColor(Color::Red);
        else
            hardText.setFillColor(Color::White);

        window.display();
    }
    return selectedLevel;
}

// Function to display the home page with buttons
void displayHomePage(RenderWindow& window, Font& font) {
    Texture homeTexture;
    if (!homeTexture.loadFromFile("texture/main page.jpg")) {
        cerr << "Error: Could not load background texture!" << endl;
    }
    Sprite homePage(homeTexture);
    homePage.setScale(
        static_cast<float>(WINDOW_WIDTH) / homePage.getLocalBounds().width,
        static_cast<float>(WINDOW_HEIGHT) / homePage.getLocalBounds().height
    );

    Text startText("START", font, 70);
    startText.setPosition(880, 750);
    startText.setFillColor(Color::Red);

    Text optionsText("OPTIONS", font, 70);
    optionsText.setPosition(850, 850);
    optionsText.setFillColor(Color::White);

    Text exitText("EXIT", font, 70);
    exitText.setPosition(900, 950);
    exitText.setFillColor(Color::White);

    SoundBuffer navigationBuffer, selectionBuffer;
    Sound navigationSound, selectionSound;

    if (!navigationBuffer.loadFromFile("texture/navigation.mp3") ||
        !selectionBuffer.loadFromFile("texture/selection.mp3")) {
        cerr << "Error loading sound files for navigation or selection!" << endl;
    }

    navigationSound.setBuffer(navigationBuffer);
    selectionSound.setBuffer(selectionBuffer);

    int selectedOption = 0;
    bool selectingMain = true;

    while (selectingMain) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Escape) {
                    window.close();
                }
                if (event.key.code == Keyboard::Up) {
                    selectedOption = (selectedOption - 1 + 3) % 3;
                    navigationSound.play();
                }
                if (event.key.code == Keyboard::Down) {
                    selectedOption = (selectedOption + 1) % 3;
                    navigationSound.play();
                }
                if (event.key.code == Keyboard::Enter) {
                    switch (selectedOption) {
                    case 0:
                        selectionSound.play();
                        selectingMain = false;
                        break;
                    case 1:
                        selectionSound.play();
                        displayOptionsMenu(window, font, soundEnabled);
                        break;
                    case 2:
                        selectionSound.play();
                        window.close();
                        break;
                    }
                }
            }
        }

        startText.setFillColor(selectedOption == 0 ? Color::Red : Color::White);
        optionsText.setFillColor(selectedOption == 1 ? Color::Red : Color::White);
        exitText.setFillColor(selectedOption == 2 ? Color::Red : Color::White);

        // Render home page
        window.clear();
        window.draw(homePage);
        window.draw(startText);
        window.draw(optionsText);
        window.draw(exitText);
        window.display();
    }
}

// Function to display option menu
void displayOptionsMenu(RenderWindow& window, Font& font, bool& soundEnabled) {
    Texture optionsTexture;
    if (!optionsTexture.loadFromFile("texture/main page.jpg")) {
        cerr << "Error: Could not load options background texture!" << endl;
    }
    Sprite optionsPage(optionsTexture);
    optionsPage.setScale(
        static_cast<float>(WINDOW_WIDTH) / optionsPage.getLocalBounds().width,
        static_cast<float>(WINDOW_HEIGHT) / optionsPage.getLocalBounds().height
    );

    Text soundText("SOUND: " + string(soundEnabled ? "ON" : "OFF"), font, 70);
    soundText.setPosition(800, 750);
    soundText.setFillColor(Color::Red);

    Text backText("BACK", font, 70);
    backText.setPosition(880, 850);
    backText.setFillColor(Color::White);


    SoundBuffer navigationBuffer, selectionBuffer;
    Sound navigationSound, selectionSound;

    if (!navigationBuffer.loadFromFile("texture/navigation.mp3") ||
        !selectionBuffer.loadFromFile("texture/selection.mp3")) {
        cerr << "Error loading sound files for navigation or selection!" << endl;
    }

    navigationSound.setBuffer(navigationBuffer);
    selectionSound.setBuffer(selectionBuffer);

    bool selecting = true;
    while (selecting) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Up || event.key.code == Keyboard::Down) {
                    if (soundText.getFillColor() == Color::Red) {
                        soundText.setFillColor(Color::White);
                        backText.setFillColor(Color::Red);
                        navigationSound.play();
                    }
                    else {
                        soundText.setFillColor(Color::Red);
                        backText.setFillColor(Color::White);
                        navigationSound.play();
                    }
                }
                if (event.key.code == Keyboard::Enter) {
                    if (soundText.getFillColor() == Color::Red) {
                        soundEnabled = !soundEnabled;
                        soundText.setString("SOUND: " + string(soundEnabled ? "ON" : "OFF"));
                        selectionSound.play();
                    }
                    else if (backText.getFillColor() == Color::Red) {
                        selecting = false;
                        selectionSound.play();
                    }
                }
                if (event.key.code == Keyboard::Escape) {
                    selecting = false;
                }
            }
        }

        window.clear();
        window.draw(optionsPage);
        window.draw(soundText);
        window.draw(backText);
        window.display();
    }
}

//Function to read high score
void readHighScores(int highScores[]) {
    ifstream inFile(HIGH_SCORE_FILE);
    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            if (line.find("Easy:") == 0) {
                highScores[0] = stoi(line.substr(5));
            }
            else if (line.find("Medium:") == 0) {
                highScores[1] = stoi(line.substr(7));
            }
            else if (line.find("Hard:") == 0) {
                highScores[2] = stoi(line.substr(5));
            }
        }
        inFile.close();
    }
    else {
        for (int i = 0; i < 3; i++) {
            highScores[i] = 0;
        }
    }
}

//Function to write high score
void writeHighScores(const int highScores[]) {
    ofstream outFile(HIGH_SCORE_FILE);
    if (outFile.is_open()) {
        outFile << "Easy: " << highScores[0] << endl;
        outFile << "Medium: " << highScores[1] << endl;
        outFile << "Hard: " << highScores[2] << endl;
        outFile.close();
    }
}

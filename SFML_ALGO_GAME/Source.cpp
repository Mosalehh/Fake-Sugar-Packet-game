#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
using namespace std;

const int SIZE = 5;
const int CELL_SIZE = 100;
const int MAX_STACK_SIZE = 10000;  // Adjust based on expected depth

struct Token {
    int r, c;
};

struct GameState {
    Token playerA[SIZE - 2];
    Token playerB[SIZE - 2];
    bool isATurn;
    bool gameOver;

    GameState() : isATurn(true), gameOver(false) {
        for (int i = 1; i < SIZE - 1; i++)
            playerA[i - 1] = { i, 0 };
        for (int j = 1; j < SIZE - 1; j++)
            playerB[j - 1] = { 0, j };
    }

    bool isCorner(int r, int c) const {
        return (r == 0 || r == SIZE - 1) && (c == 0 || c == SIZE - 1);
    }

    bool isOccupied(int r, int c) const {
        for (int i = 0; i < SIZE - 2; i++) {
            if ((playerA[i].r == r && playerA[i].c == c) ||
                (playerB[i].r == r && playerB[i].c == c)) {
                return true;
            }
        }
        return false;
    }

    bool hasAWon() const {
        for (int i = 0; i < SIZE - 2; i++)
            if (playerA[i].c < SIZE - 1)
                return false;
        return true;
    }

    bool hasBWon() const {
        for (int i = 0; i < SIZE - 2; i++)
            if (playerB[i].r < SIZE - 1)
                return false;
        return true;
    }

    bool isGameOver() const {
        return hasAWon() || hasBWon();
    }

    void setGameOver(bool value) {
        gameOver = value;
    }
};

class GameStateStack {
    GameState stack[MAX_STACK_SIZE];
    int top;

public:
    GameStateStack() : top(-1) {}

    bool isEmpty() const {
        return top == -1;
    }

    bool isFull() const {
        return top == MAX_STACK_SIZE - 1;
    }

    void push(const GameState& state) {
        if (!isFull()) stack[++top] = state;
    }

    GameState pop() {
        if (!isEmpty()) return stack[top--];
        return GameState();  // Return default if empty
    }
};


GameStateStack moveStack;  // Global stack to keep track of game states

bool makeMove(GameState& state, int index, bool jump) {                                             //The Use of Stack Data Structure
    GameState originalState = state;  // Save current state to push later if move is valid

    if (state.isATurn) {
        Token& t = state.playerA[index];
        int nc = t.c + (jump ? 2 : 1);

        if (nc >= SIZE || state.isCorner(t.r, nc)) return false;
        if (jump && !state.isOccupied(t.r, t.c + 1)) return false;
        if (state.isOccupied(t.r, nc)) return false;

        t.c = nc;
    } else {
        Token& t = state.playerB[index];
        int nr = t.r + (jump ? 2 : 1);

        if (nr >= SIZE || state.isCorner(nr, t.c)) return false;
        if (jump && !state.isOccupied(t.r + 1, t.c)) return false;
        if (state.isOccupied(nr, t.c)) return false;

        t.r = nr;
    }

    moveStack.push(originalState);  // Push the valid previous state to the stack
    state.isATurn = !state.isATurn;
    return true;
}

string PlayAnyGame(GameState state, bool isATurn) {        //Evaluate the move                                  //BACKTRACKING 
    if (state.hasAWon()) return "good";
    if (state.hasBWon()) return "bad";

    for (int i = 0; i < SIZE - 2; i++) {
        for (int jump = 0; jump <= 1; jump++) {
            GameState next = state;
            if (!makeMove(next, i, jump)) continue;
            if (isATurn && !next.isATurn) continue; // Skip if not player's turn
            string result = PlayAnyGame(next, !isATurn);
            if (result == "bad") {
                return "good";
            }
        }
    }

    return "bad";
}

bool botMove(GameState& state) {
    for (int i = 0; i < SIZE - 2; i++) {
        for (int jump = 0; jump <= 1; jump++) {
            GameState temp = state;
            if (!makeMove(temp, i, jump)) continue;
            if (!temp.isATurn) continue;

            // Use PlayAnyGame to evaluate if this move is successful
            if (PlayAnyGame(temp, false) == "good") {
                makeMove(state, i, jump);
                return true;
            }
        }
    }

    // If no winning move, make any valid move
    for (int i = 0; i < SIZE - 2; i++) {
        for (int jump = 0; jump <= 1; jump++) {
            if (makeMove(state, i, jump)) return true;
        }
    }

    return false;
}

void drawBoard(sf::RenderWindow& window, GameState& state, sf::Font& font) {
    // Load the images for player tokens
    sf::Texture playerATexture, playerBTexture;
    if (!playerATexture.loadFromFile("resources/playerA.png") || !playerBTexture.loadFromFile("resources/playerB.png")) {
        std::cerr << "Error loading images!" << std::endl;
        return;
    }

    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
            cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);
            cell.setFillColor(state.isCorner(i, j) ? sf::Color(128, 128, 128) : sf::Color::White);
            cell.setOutlineThickness(2);
            cell.setOutlineColor(sf::Color::Black);
            window.draw(cell);
        }
    }

    // Draw player A's tokens using playerA.png
    for (int i = 0; i < SIZE - 2; i++) {
        sf::Sprite playerASprite(playerATexture);
        playerASprite.setScale(0.3f, 0.3f);  // Scale image to fit inside the cell
        playerASprite.setPosition(state.playerA[i].c * CELL_SIZE + 10, state.playerA[i].r * CELL_SIZE + 10);
        window.draw(playerASprite);
    }

    // Draw player B's tokens using playerB.png
    for (int i = 0; i < SIZE - 2; i++) {
        sf::Sprite playerBSprite(playerBTexture);
        playerBSprite.setScale(0.3f, 0.3f);  // Scale image to fit inside the cell
        playerBSprite.setPosition(state.playerB[i].c * CELL_SIZE + 10, state.playerB[i].r * CELL_SIZE + 10);
        window.draw(playerBSprite);
    }

    // Clear previous win text before displaying the new one
    if (state.gameOver) {
        window.close();  // Close the current game window

        // Create a new window to show the winner
        sf::RenderWindow resultWindow(sf::VideoMode(400, 200), "Game Over");

        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(32);
        text.setFillColor(sf::Color::Black);
        text.setPosition(20, 80);
        text.setString(state.hasAWon() ? "Player A Wins!" : "Bot B Wins!");

        // Show the result window until the user closes it
        while (resultWindow.isOpen()) {
            sf::Event event;
            while (resultWindow.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    resultWindow.close();
            }

            resultWindow.clear(sf::Color::White);
            resultWindow.draw(text);
            resultWindow.display();
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(SIZE * CELL_SIZE, SIZE * CELL_SIZE + 100), "Fake Sugar Packet Game");
    sf::Font font;
    font.loadFromFile("resources/ARIAL.TTF");

    GameState game;
    int selectedToken = 0;
    bool jumpMove = false;

    sf::Text instruction;
    instruction.setFont(font);
    instruction.setCharacterSize(17);
    instruction.setFillColor(sf::Color::Black);
    instruction.setPosition(10, SIZE * CELL_SIZE + 10);
    instruction.setString("Use number keys 0-2 to select token, J for jump, M to move.");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (!game.gameOver && game.isATurn && event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Num0) selectedToken = 0;
                if (event.key.code == sf::Keyboard::Num1) selectedToken = 1;
                if (event.key.code == sf::Keyboard::Num2) selectedToken = 2;

                if (event.key.code == sf::Keyboard::J) jumpMove = true;
                if (event.key.code == sf::Keyboard::M) {
                    bool moved = makeMove(game, selectedToken, jumpMove);
                    jumpMove = false; // Always reset after an attempt, successful or not
                    if (moved && !game.isATurn) {
                        botMove(game);
                    }
                }
            }
        }

        window.clear(sf::Color::White);
        drawBoard(window, game, font);

        // Draw instruction and status of the game
        window.draw(instruction);

        sf::Text status;
        status.setFont(font);
        status.setCharacterSize(17);
        status.setFillColor(sf::Color::Black);
        status.setPosition(10, SIZE * CELL_SIZE + 40);
        std::stringstream ss;
        ss << "Selected Token: " << selectedToken << " | Jump: " << (jumpMove ? "ON" : "OFF");
        status.setString(ss.str());
        window.draw(status);

        // Check if the game is over
        if (game.isGameOver()) {
            game.setGameOver(true);  // Mark the game as over once a player wins
        }

        window.display();
    }

    return 0;
}


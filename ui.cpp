#include <SFML/Graphics.hpp>
#include <iostream>
#include <utility>
#include <memory>
#include "Board.h"
#include "Helper.h"


float FIELD_HEIGHT = 48.5;
float FIELD_WIDTH = 48.5;
float FIELD_START_X = 35;
float FIELD_START_Y = 40;
float FIELD_CENTER_OFFSET_X = FIELD_WIDTH / 2;
float FIELD_CENTER_OFFSET_Y = FIELD_HEIGHT / 2;

std::vector<std::string> texture_names {
    "../textures/Black_Rook.png",
    "../textures/Black_Knight.png",
    "../textures/Black_Bishop.png",
    "../textures/Black_Queen.png",
    "../textures/Black_King.png",
    "../textures/Black_Pawn.png",
    "../textures/White_Rook.png",
    "../textures/White_Knight.png",
    "../textures/White_Bishop.png",
    "../textures/White_Queen.png",
    "../textures/White_King.png",
    "../textures/White_Pawn.png"
};

int8_t** internal_board;

void set_sprite_position(std::shared_ptr<sf::Sprite>& sprite, float x, float y) {
    sprite->setPosition(x - FIELD_CENTER_OFFSET_X, y - FIELD_CENTER_OFFSET_Y);
}

int x_coord_to_ind(float x, bool center = false) {
    return (x - FIELD_START_X - (center ? FIELD_CENTER_OFFSET_X : 0)) / FIELD_WIDTH;
}

int y_coord_to_ind(float y, bool center = false) {
    return (y - FIELD_START_Y - (center ? FIELD_CENTER_OFFSET_Y : 0)) / FIELD_HEIGHT;
}

std::pair<int, int> coords_to_indices(float x, float y, bool center = false) {
    return std::make_pair(x_coord_to_ind(x, center), y_coord_to_ind(y, center));
}

void setup_board(sf::RenderWindow& window, 
                std::pair<float, float> board_coords[8][8], 
                std::map<std::pair<float, float>, std::shared_ptr<sf::Sprite>>& sprites,
                std::map<std::string, sf::Texture>& textures) 
{
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
            std::string texture_name = "";
            switch (internal_board[i][j]) {
                case -6:
                    texture_name = "../textures/Black_Rook.png";
                    break;
                case -3:
                    texture_name = "../textures/Black_Knight.png";
                    break;
                case -4:
                    texture_name = "../textures/Black_Bishop.png";
                    break;
                case -9:
                    texture_name = "../textures/Black_Queen.png";
                    break;
                case -63:
                    texture_name = "../textures/Black_King.png";
                    break;
                case -1:
                    texture_name = "../textures/Black_Pawn.png";
                    break;
                case 6:
                    texture_name = "../textures/White_Rook.png";
                    break;
                case 3:
                    texture_name = "../textures/White_Knight.png";
                    break;
                case 4:
                    texture_name = "../textures/White_Bishop.png";
                    break;
                case 9:
                    texture_name = "../textures/White_Queen.png";
                    break;
                case 63:
                    texture_name = "../textures/White_King.png";
                    break;
                case 1:
                    texture_name = "../textures/White_Pawn.png";
                    break;
                default:
                    break;
            }

            if (texture_name != "") {
                std::shared_ptr<sf::Sprite> sprite = std::make_shared<sf::Sprite>(textures[texture_name]);
                set_sprite_position(sprite, board_coords[j][i].first, board_coords[j][i].second);
                window.draw(*sprite);

                auto [cx, cy] = coords_to_indices(board_coords[j][i].first, board_coords[j][i].second, true);
                sprites[std::make_pair(cx, cy)] = sprite;

            }
        }
    }
}

std::pair<float, float> get_nearest_valid_position(float cur_x, float cur_y, std::vector<float>& x_coords_center_field, std::vector<float>& y_coords_center_field) {
    auto nearest_valid_x_center = *std::min_element(x_coords_center_field.begin(), x_coords_center_field.end(), [cur_x](int a, int b) {
        return std::abs(a - cur_x) < std::abs(b - cur_x);
    });
    auto nearest_valid_y_center = *std::min_element(y_coords_center_field.begin(), y_coords_center_field.end(), [cur_y](int a, int b) {
        return std::abs(a - cur_y) < std::abs(b - cur_y);
    });
    return std::make_pair(nearest_valid_x_center, nearest_valid_y_center);
}

int main()
{

    Board b = Board();
    internal_board = b.board;

    std::pair<float, float> board_coords[8][8];
    std::vector<float> x_coords_center_field;
    std::vector<float> y_coords_center_field;


    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
            board_coords[i][j] = std::make_pair(FIELD_CENTER_OFFSET_X + FIELD_START_X + FIELD_WIDTH * i, 
                                                FIELD_CENTER_OFFSET_Y + FIELD_START_Y + FIELD_HEIGHT * j);
            x_coords_center_field.push_back(FIELD_CENTER_OFFSET_X + FIELD_START_X + FIELD_WIDTH * i);
            y_coords_center_field.push_back(FIELD_CENTER_OFFSET_Y + FIELD_START_Y + FIELD_HEIGHT * j);
        }
    }

    sf::RenderWindow window(sf::VideoMode(458, 468), "Chess AI", sf::Style::Titlebar | sf::Style::Close);
    
    // Load the chess board
    sf::Texture chess_board;
    if (!chess_board.loadFromFile("../textures/Chess_Board.png")) {
        return -1; // Error handling
    }
    sf::Sprite board_sprite(chess_board);

    std::shared_ptr<sf::Sprite> chosen_sprite = nullptr;
    std::map<std::pair<float, float>, std::shared_ptr<sf::Sprite>> sprites;
    bool is_dragging = false;
    float x_before_drag, y_before_drag;
    int pickup_x_index, pickup_y_index;
    sf::Vector2f offset;

    std::map<std::string, sf::Texture> textures;
    for (auto texture_name : texture_names) {
        sf::Texture texture;
        if (!texture.loadFromFile(texture_name)) {
            return -1; // Error handling
        }
        textures[texture_name] = texture;
    }

    setup_board(window, board_coords, sprites, textures);

    bool own_move = true;

    while (window.isOpen())
    {

        // i hate this, just do random piece
        auto moves = b.getAllLegalMoves(own_move);

        if(moves.empty() || b.piecesLeft <= 2) {
            break;
        }

        if (!own_move) {
            auto bestMove = b.bestMove(4, own_move, INT_MIN, INT_MAX);
            uint32_t performingMove = bestMove.first.top();
            int8_t x1 = performingMove >> 24;
            int8_t y1 = (performingMove & 0xff0000) >> 16;
            int8_t x2 = (performingMove & 0xff00) >> 8;
            int8_t y2 = performingMove & 0xff;

            b.executeMove(x1, y1, x2, y2);
            own_move = true;
            sprites.clear();

            setup_board(window, board_coords, sprites, textures);

        } else {
            sf::Event event;
            while (window.pollEvent(event)) {

                // TODO allow closing when AI is thinking
                if (event.type == sf::Event::Closed) {
                    window.close();
                }

                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        sf::Vector2i mouse_position = sf::Mouse::getPosition(window);

                        auto [nearest_valid_x_center, nearest_valid_y_center] = get_nearest_valid_position(mouse_position.x, mouse_position.y, x_coords_center_field, y_coords_center_field);
                        auto [coord_x, coord_y] = coords_to_indices(nearest_valid_x_center, nearest_valid_y_center, true);

                        // This check should be sufficient since internal and map should always be in sync
                        if (internal_board[coord_y][coord_x] == '\x00') {
                            std::cout << "No piece at this position (internal representation)\n" << chosen_sprite << "\n";
                            continue;
                        }
                        pickup_x_index = coord_x;
                        pickup_y_index = coord_y;

                        chosen_sprite = sprites[std::make_pair(coord_x, coord_y)];
                        if (chosen_sprite == nullptr) {
                            continue;
                        }

                        sf::FloatRect spriteBounds = chosen_sprite->getGlobalBounds();

                        if (spriteBounds.contains(static_cast<sf::Vector2f>(mouse_position))) {
                            is_dragging = true;
                            x_before_drag = chosen_sprite->getPosition().x;
                            y_before_drag = chosen_sprite->getPosition().y;
                            offset = chosen_sprite->getPosition() - static_cast<sf::Vector2f>(mouse_position);
                        }
                    }
                    else {
                        int s = 0;
                        Helper::printBoard(internal_board);
                        for (auto [key, sprite] : sprites) {
                            if (sprite != nullptr) {
                                s++;
                            }
                        }
                        std::cout << s << "\n";
                        window.clear();
                    }
                }

                if (event.type == sf::Event::MouseButtonReleased && chosen_sprite != nullptr) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        is_dragging = false;

                        // Find nearest valid x and y position
                        auto current_pos = sf::Mouse::getPosition(window);
                        auto cur_x = current_pos.x;
                        auto cur_y = current_pos.y;
                        auto [nearest_valid_x_center, nearest_valid_y_center] = get_nearest_valid_position(cur_x, cur_y, x_coords_center_field, y_coords_center_field);
                        auto [coord_x, coord_y] = coords_to_indices(nearest_valid_x_center, nearest_valid_y_center, true);

                        // Check if move is valid TODO
                        // if (is_valid_move(pickup_x_index, pickup_y_index, coord_x, coord_y)) {
                        //     set_sprite_position(chosen_sprite, nearest_valid_x_center, nearest_valid_y_center);
                        //     sprites[std::make_pair(pickup_x_index, pickup_y_index)] = nullptr;
                        //     sprites[std::make_pair(coord_x, coord_y)] = chosen_sprite;
                        //     internal_move(pickup_x_index, pickup_y_index, coord_x, coord_y);
                        // } else {
                        //     chosen_sprite->setPosition(x_before_drag, y_before_drag);
                        // }
                        bool executedMove = b.parseMove(pickup_y_index, pickup_x_index, coord_y, coord_x);
                        if(executedMove) {
                            set_sprite_position(chosen_sprite, nearest_valid_x_center, nearest_valid_y_center);
                            sprites[std::make_pair(pickup_x_index, pickup_y_index)] = nullptr;
                            sprites[std::make_pair(coord_x, coord_y)] = chosen_sprite;
                            std::cout << "Moving from " << char(pickup_x_index + 'a') << " " << (8-pickup_y_index) << " to " << char(coord_x + 'a') << " " << (8-coord_y) << "\n";
                            std::cout << "Moving from " << pickup_x_index << " " << pickup_y_index << " to " << coord_x << " " << coord_y << "\n";
                            own_move = false;
                            chosen_sprite = nullptr;
                        } else {
                            chosen_sprite->setPosition(x_before_drag, y_before_drag);
                        }
                    }
                }
            }

            if (is_dragging && chosen_sprite != nullptr) {
                sf::Vector2i mouse_position = sf::Mouse::getPosition(window);
                chosen_sprite->setPosition(static_cast<sf::Vector2f>(mouse_position) + offset);
            }


        }

        window.clear();
        window.draw(board_sprite);
        for (auto [key, sprite] : sprites) {
            if (sprite != nullptr) {
                window.draw(*sprite);
            }
        }
        window.display();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    return 0;
}

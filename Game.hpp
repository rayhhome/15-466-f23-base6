#pragma once

#include <cstdint>
#include <glm/glm.hpp>

#include <string>
#include <list>
#include <random>

struct Connection;

//Game state, separate from rendering.

//Currently set up for a "client sends controls" / "server sends whole state" situation.

enum class Message : uint8_t {
	C2S_Controls = 1, //Greg!
	S2C_State = 's',
	//...
};

//used to represent a control input:
struct Button {
	uint8_t downs = 0; //times the button has been pressed
	bool pressed = false; //is the button pressed now
};

//state of one player in the game:
struct Player {
	//player inputs (sent from client):
	struct Controls {
		Button left, right, up, down, jump;

		void send_controls_message(Connection *connection) const;

		//returns 'false' if no message or not a controls message,
		//returns 'true' if read a controls message,
		//throws on malformed controls message
		bool recv_controls_message(Connection *connection);
	} controls;

	//player state (sent from server):
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
	glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
	glm::vec2 direction = glm::vec2(1.0f, 0.0f);

	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	float jump_cooldown = 0.0f;
	std::string name = "";
};

//state of one touch element:
struct Element {
	glm::vec2 position = glm::vec2(0.0f, 0.0f);
	glm::vec2 velocity = glm::vec2(0.0f, 0.0f);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct Game {
	std::list< Player > players; //(using list so they can have stable addresses)
	std::vector< Element > elements; //(using vector so they can be indexed)
	Player *spawn_player(); //add player the end of the players list (may also, e.g., play some spawn anim)
	void remove_player(Player *); //remove player from game (may also, e.g., play some despawn anim)
	Element *spawn_element();

	std::mt19937 mt; //used for spawning players
	uint32_t next_player_number = 1; //used for naming players
	uint32_t team1_points = 0;
	uint32_t team2_points = 0;

	Game();

	//state update function:
	void update(float elapsed);

	//constants:
	//the update rate on the server:
	inline static constexpr float Tick = 1.0f / 30.0f;

	//arena size:
	inline static constexpr glm::vec2 ArenaMin = glm::vec2(-1.0f, -1.0f);
	inline static constexpr glm::vec2 ArenaMax = glm::vec2( 1.00f,  1.0f);

	//player constants:
	inline static constexpr float PlayerRadius = 0.06f;
	inline static constexpr float PlayerSpeed = 1.25f;
	inline static constexpr float PlayerAccelHalflife = 0.25f;
	inline static constexpr float PlayerJumpSpeed = 2.0f;
	inline static constexpr float PlayerCoolDown = 1.0f;
	inline static constexpr glm::vec3 PlayerColor1 = glm::vec3(1.0f, 0.0f, 1.0f);
	inline static constexpr glm::vec3 PlayerColor2 = glm::vec3(0.0f, 1.0f, 1.0f);

	//touch element constants:
	inline static constexpr float ElementRadius = 0.05f;
	inline static constexpr float ElementAccelHalflife = 0.2f;

	//---- communication helpers ----

	//used by client:
	//set game state from data in connection buffer
	// (return true if data was read)
	bool recv_state_message(Connection *connection);

	//used by server:
	//send game state.
	//  Will move "connection_player" to the front of the front of the sent list.
	void send_state_message(Connection *connection, Player *connection_player = nullptr) const;
};

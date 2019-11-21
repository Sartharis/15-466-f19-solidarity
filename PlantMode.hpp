#pragma once

#include "Mode.hpp"

#include "BoneAnimation.hpp"
#include "GL.hpp"
#include "Scene.hpp"
#include "Order.hpp"
#include "Button.hpp"
#include "UIElem.hpp"

#include <SDL.h>
#include <glm/glm.hpp>

#include <vector>
#include <list>
#include <unordered_map>
#include <iostream>
#include "Plant.hpp"

struct Inventory
{ // NOTE: should make sure to NEVER INSERT NULL INTO THE MAP!!! AAAAAAHHHH
	// TODO: when seed / harvest number changes, update UI by changing corresponding elem text and re-layout all plant elems.
	int get_seeds_num( const PlantType* plant );
	void change_seeds_num(const PlantType* plant, int seed_change );
	int get_harvest_num( const PlantType* plant );
	void change_harvest_num(const PlantType* plant, int harvest_change );

	UIElem* get_seed_item( const PlantType* plant );
	void set_seed_item( const PlantType* plant, UIElem* item ) { plant_to_seed_item.insert( std::make_pair( plant, item ) ); }
	UIElem* get_harvest_item( const PlantType* plant );
	void set_harvest_item( const PlantType* plant, UIElem* item ) { plant_to_harvest_item.insert( std::make_pair( plant, item ) ); }

	static bool comp_fn(std::pair<PlantType const*, int> p1, std::pair<PlantType const*, int> p2) {
		return p1.second > p2.second;
	} // use this to sort entries in descending order

	//getters
	std::unordered_map<PlantType const*, int> get_plant_to_seeds() { return plant_to_seeds; }
	std::unordered_map<PlantType const*, int> get_plant_to_harvest() { return plant_to_harvest; }
private:
	std::unordered_map<PlantType const*, int> plant_to_seeds;
	std::unordered_map<PlantType const*, int> plant_to_harvest;
	std::unordered_map<PlantType const*, Button*> plant_to_seed_btn;
	std::unordered_map<PlantType const*, Button*> plant_to_harvest_btn;

	std::unordered_map<PlantType const*, UIElem*> plant_to_seed_item;
	std::unordered_map<PlantType const*, UIElem*> plant_to_harvest_item;
};

// The 'PlantMode':
struct PlantMode : public Mode {
	PlantMode();
	virtual ~PlantMode();

	int current_order_idx = 0;
	OrderType const* current_order = nullptr;
	bool cancel_order_state = false;
	float cancel_order_freeze_time = 10;
	int current_main_order_idx = 0;
	OrderType const* current_main_order = nullptr;
	
	// init harvest_plant_map
	// Harvest Plant Map
	std::map< PlantType const*, int > harvest_plant_map;
    
	void on_click( int x, int y );
	GroundTile* get_tile_under_mouse( int x, int y);
	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	virtual void on_resize( glm::uvec2 const& new_drawable_size ) override;
	
	//scene:
	std::string tool_name = "";
	std::string tool_description = "";
	std::string action_description = "";
	const PlantType* selectedPlant = nullptr;
	const PlantType* orderPlant = nullptr;
	Scene scene;
	Scene::Camera *camera = nullptr;
	Scene::Drawable* selector = nullptr;
	
	Inventory inventory;
	int num_coins = 30;
	void change_num_coins(int change);

	glm::vec3 forward_camera_dir = glm::vec3();
	glm::vec3 forward_dir = glm::vec3();

	glm::vec3 side_camera_dir = glm::vec3();
	glm::vec3 side_dir = glm::vec3();

	glm::vec3 camera_bounds_min = glm::vec3(-5.0f, -5.0f,-10000.0f);
	glm::vec3 camera_bounds_max = glm::vec3( 5.0f, 5.0f, 10000.0f );
	glm::vec3 camera_offset = glm::vec3();
	float camera_move_speed = 2.0f;
	float camera_radius = 7.5f;
	float camera_azimuth = glm::radians(125.0f);
	float camera_elevation = glm::radians(40.0f);

	//tool selection
	Tool current_tool = default_hand;
	void set_current_tool(Tool tool);

	//UI states:
	void setup_UI();
	struct {
		UIElem* root;
		UIElem* coins_text;
		struct {
			UIElem* glove;
			UIElem* watering_can;
			UIElem* fertilizer;
			UIElem* shovel;
		} toolbar;
		// storage
		int storage_current_tab = 0;

		// NOTE: deprecated, to be removed
		std::vector< Button* > all_buttons = {};
		// order
		struct {
			glm::vec2 tr_offset = glm::vec2(-360.0f, 150.0f);
		} order;
	} UI;

	UIElem* UI_root = nullptr;

	struct {
		Sprite const* sprite = nullptr;
		std::string text = ""; // TODO: text that floats around cursor?
		float scale = 1.0f;
		glm::vec2 offset = glm::vec2(0, 0);// applied to cursor sprite _before_ scaling
	} cursor;

	//-------- opengl stuff 

	glm::vec2 screen_size = glm::vec2(960, 600); 
	GLuint color_attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	GLuint firstpass_fbo = 0;
	GLuint firstpass_color_attachments[2]; 
	GLuint firstpass_depth_attachment = 0;

	GLuint aura_fbo = 0; // shares depth attachment with firstpass_fbo
	GLuint aura_color_attachment = 0;

	GLuint pingpong_fbos[2];
	GLuint pingpong_color_attachments[2];

	std::vector<float> trivial_vector = {
		-1, -1, 0,
		-1, 1, 0,
		1, 1, 0,
		-1, -1, 0,
		1, 1, 0,
		1, -1, 0
	};

	GLuint trivial_vao = 0;
	GLuint trivial_vbo = 0;

	//--------
};

extern Load< SpriteAtlas > main_atlas;
extern Sprite const* order_background_sprite;

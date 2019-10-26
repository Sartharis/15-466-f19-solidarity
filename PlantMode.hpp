#pragma once

#include "Mode.hpp"

#include "BoneAnimation.hpp"
#include "GL.hpp"
#include "Scene.hpp"

#include <SDL.h>
#include <glm/glm.hpp>

#include <vector>
#include <list>

/* Contains info on how a plant works and looks like*/
struct PlantType
{
	PlantType( const Mesh* mesh_in,
			   int cost_in = 5,
			   int harvest_gain_in = 7,
			   float growth_time_in = 5.0f, 
			   std::string name_in = "Default Name", 
			   std::string description_in = "Default Description." )
	 : mesh( mesh_in ), 
		cost(cost_in), 
		harvest_gain(harvest_gain_in),
		growth_time(growth_time_in), 
		name(name_in), 
		description(description_in)  {};

	const Mesh* get_mesh() const { return mesh; };
	float get_growth_time() const { return growth_time; };
	int get_cost() const { return cost; };
	int get_harvest_gain() const { return harvest_gain; };
	std::string get_name() const { return name; };
	std::string get_description() const { return description; };

private:
	const Mesh* mesh = nullptr;
	float growth_time = 5.0f;
	int cost = 5;
	int harvest_gain = 7;
	std::string name = "Default Name";
	std::string description = "Default Description.";
};

/* Contains info on how a tile works and looks like*/
struct GroundTileType
{
	GroundTileType( bool can_plant_in, const Mesh* tile_mesh_in ) : can_plant( can_plant_in ), mesh( tile_mesh_in ){};
	const Mesh* get_mesh() const{ return mesh; };
	bool get_can_plant() const { return can_plant; };

private:
	bool can_plant = true;
	const Mesh* mesh = nullptr;
};

/* Actual instance of a tile with a plant */
struct GroundTile
{
	void change_tile_type( const GroundTileType* tile_type_in );
	void update( float elapsed );
	void update_plant_visuals( float percent_grown );
	bool try_add_plant(const PlantType* plant_type_in );
	bool try_remove_plant();
	bool is_tile_harvestable();

	// Tile and plant types
	const GroundTileType* tile_type = nullptr;
	const PlantType* plant_type = nullptr;
	Scene::Drawable* tile_drawable = nullptr;
	Scene::Drawable* plant_drawable = nullptr;

	// Tile data
	int grid_x = 0;
	int grid_y = 0;

	// Plant data
	float current_grow_time = 0.0f;

	//TEMP!!!!!
	float start_height = -0.4f;
	float end_height = 0.0f;
};

// The 'PlantMode':
struct PlantMode : public Mode {
	PlantMode();
	virtual ~PlantMode();

	void on_click( int x, int y );
	GroundTile* get_tile_under_mouse( int x, int y);
	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//scene:
	std::string action_description = "";
	const PlantType* selectedPlant = nullptr;
	Scene scene;
	Scene::Camera *camera = nullptr;

	GroundTile** grid = nullptr;
	glm::vec2 plant_grid_tile_size = glm::vec2( 1.0f, 1.0f );
	int plant_grid_x = 20;
	int plant_grid_y = 20;

	int energy = 20;

	float camera_radius = 7.5f;
	float camera_azimuth = glm::radians(90.0f);
	float camera_elevation = glm::radians(45.0f);

	//-------- opengl stuff 

	// TODO: if want to allow resize, have to find a better way to pass this
	glm::vec2 screen_size = glm::vec2(960, 600); 

	GLuint firstpass_fbo = 0;
	GLuint colorBuffers[2];
	GLuint depthBuffer = 0;
	GLuint color_attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	GLuint pingpong_fbo[2];
	GLuint pingpongBuffers[2];

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

#include "Plant.hpp"
#include "FirstpassProgram.hpp"
#include "PostprocessingProgram.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include <cstddef>
#include <algorithm>
#include <iostream>

const MeshBuffer* plant_mesh_buffer;
glm::vec2 plant_grid_tile_size = glm::vec2( 1.0f, 1.0f );
PlantType const* test_plant = nullptr;
PlantType const* friend_plant = nullptr;
PlantType const* vampire_plant = nullptr;
PlantType const* cactus_plant = nullptr;
PlantType const* fireflower_plant = nullptr;
PlantType const* corpseeater_plant = nullptr;
GroundTileType const* sea_tile = nullptr;
GroundTileType const* ground_tile = nullptr;
GroundTileType const* dirt_tile = nullptr;
GroundTileType const* grass_short_tile = nullptr;
GroundTileType const* grass_tall_tile = nullptr;

// ground tiles
Mesh const* sea_tile_mesh = nullptr;
Mesh const* ground_tile_mesh = nullptr;
Mesh const* dirt_tile_mesh = nullptr;
Mesh const* grass_short_tile_mesh = nullptr;
Mesh const* grass_tall_tile_mesh = nullptr;

// Dead plant
Mesh const* dead_plant_mesh = nullptr;

// test plant (fern)
Mesh const* test_plant_1_mesh = nullptr;
Mesh const* test_plant_2_mesh = nullptr;
Sprite const* fern_seed_sprite = nullptr;
Sprite const* fern_harvest_sprite = nullptr;
// friend plant
Mesh const* friend_plant_1_mesh = nullptr;
Mesh const* friend_plant_2_mesh = nullptr;
Mesh const* friend_plant_3_mesh = nullptr;
Sprite const* friend_plant_seed_sprite = nullptr;
Sprite const* friend_plant_harvest_sprite = nullptr;
// vampire plant
Mesh const* vampire_plant_1_mesh = nullptr;
Mesh const* vampire_plant_2_mesh = nullptr;
Mesh const* vampire_plant_3_mesh = nullptr;
Sprite const* vampire_plant_seed_sprite = nullptr;
Sprite const* vampire_plant_harvest_sprite = nullptr;
// cactus
Mesh const* cactus_1_mesh = nullptr;
Mesh const* cactus_2_mesh = nullptr;
Mesh const* cactus_3_mesh = nullptr;
Sprite const* cactus_seed_sprite = nullptr;
Sprite const* cactus_harvest_sprite = nullptr;
// fireflower
Mesh const* fireflower_1_mesh = nullptr;
Mesh const* fireflower_2_mesh = nullptr;
Mesh const* fireflower_3_mesh = nullptr;
Sprite const* fireflower_seed_sprite = nullptr;
Sprite const* fireflower_harvest_sprite = nullptr;
// corpse eater
Mesh const* corpseeater_1_mesh = nullptr;
Mesh const* corpseeater_2_mesh = nullptr;
Mesh const* corpseeater_3_mesh = nullptr;
Sprite const* corpseeater_seed_sprite = nullptr;
Sprite const* corpseeater_harvest_sprite = nullptr;

Load< SpriteAtlas > plants_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	SpriteAtlas const *ret = new SpriteAtlas(data_path("plants"));
	std::cout << "----2D sprites loaded (plants):" << std::endl;
	for( auto p : ret->sprites ) {
		std::cout << p.first << std::endl;
	}
	fern_seed_sprite = &ret->lookup( "seed1" );
	fern_harvest_sprite = &ret->lookup( "seed2" );
	friend_plant_seed_sprite = &ret->lookup( "seed1" );
	friend_plant_harvest_sprite = &ret->lookup( "seed2" );
	vampire_plant_seed_sprite = &ret->lookup( "seed1" );
	vampire_plant_harvest_sprite = &ret->lookup( "seed2" );
	cactus_seed_sprite = &ret->lookup( "seed1" );
	cactus_harvest_sprite = &ret->lookup( "seed2" );
	fireflower_seed_sprite = &ret->lookup( "seed1" );
	fireflower_harvest_sprite = &ret->lookup( "seed2" );
	corpseeater_seed_sprite = &ret->lookup( "seed1" );
	corpseeater_harvest_sprite = &ret->lookup( "seed2" );
	return ret;
});

Load< MeshBuffer > plant_meshes( LoadTagDefault, [](){
	auto ret = new MeshBuffer( data_path( "solidarity.pnct" ) );
	std::cout << "----meshes loaded:" << std::endl;
	for( auto p : ret->meshes ) {
		std::cout << p.first << std::endl;
	}

	// TILE MESHES --------------------------------------------------
	sea_tile_mesh = &ret->lookup( "sea" );
	ground_tile_mesh = &ret->lookup( "soil" );
	dirt_tile_mesh = &ret->lookup( "unoccupied" );
	grass_short_tile_mesh = &ret->lookup( "shortgrass" );
	grass_tall_tile_mesh = &ret->lookup( "tallgrass" );

	sea_tile = new GroundTileType( false, sea_tile_mesh, -1 );
	ground_tile = new GroundTileType( true, ground_tile_mesh, -1 );
	dirt_tile = new GroundTileType( false, dirt_tile_mesh, 40 );
	grass_short_tile = new GroundTileType( false, grass_short_tile_mesh, 50 );
	grass_tall_tile = new GroundTileType( false, grass_tall_tile_mesh, 60 );

	// PLANT MESHES -------------------------------------------------
	dead_plant_mesh = &ret->lookup( "deadplant" );
	test_plant_1_mesh = &ret->lookup( "leaf1" ); 
	test_plant_2_mesh = &ret->lookup( "leaf2" ); 
	friend_plant_1_mesh = &ret->lookup( "carrot1" );
	friend_plant_2_mesh = &ret->lookup( "carrot2" ); 
	friend_plant_3_mesh = &ret->lookup( "carrot3" ); 
	vampire_plant_1_mesh = &ret->lookup( "sapsucker1" ); 
	vampire_plant_2_mesh = &ret->lookup( "sapsucker2" ); 
	vampire_plant_3_mesh = &ret->lookup( "sapsucker3" );
	cactus_1_mesh = &ret->lookup( "cactus1" );
	cactus_2_mesh = &ret->lookup( "cactus2" );
	cactus_3_mesh = &ret->lookup( "cactus3" );
	fireflower_1_mesh = &ret->lookup( "fireflower1" );
	fireflower_2_mesh = &ret->lookup( "fireflower2" );
	fireflower_3_mesh = &ret->lookup( "fireflower3" );
	corpseeater_1_mesh = &ret->lookup( "leaf1" );
	corpseeater_2_mesh = &ret->lookup( "leaf2" );
	corpseeater_3_mesh = &ret->lookup( "leaf3" );

	test_plant = new PlantType( { test_plant_1_mesh, test_plant_2_mesh }, fern_seed_sprite, fern_harvest_sprite, Aura::none, 5, 10, 20.0f, "Familiar Fern", "Cheap plant. Grows anywhere." );
	friend_plant = new PlantType( { friend_plant_1_mesh, friend_plant_2_mesh, friend_plant_3_mesh }, friend_plant_seed_sprite, friend_plant_harvest_sprite, Aura::none, 10, 25, 30.0f, "Companion Carrot", "Speeds up growth of neighbors. Needs 2 neighbors to grow." );
	vampire_plant = new PlantType( { vampire_plant_1_mesh, vampire_plant_2_mesh, vampire_plant_3_mesh },vampire_plant_seed_sprite, vampire_plant_harvest_sprite, Aura::none, 20, 60, 50.0f, "Sap Sucker", "Grows by stealing life from neighbor plants. 3 plants sustain it." );
	cactus_plant = new PlantType( { cactus_1_mesh, cactus_2_mesh, cactus_3_mesh }, cactus_seed_sprite, cactus_harvest_sprite, Aura::none, 10, 20, 60.0f, "Crisp Cactus", "Grows only in fire aura from fire flowers." );
	fireflower_plant = new PlantType( { fireflower_1_mesh, fireflower_2_mesh, fireflower_3_mesh }, fireflower_seed_sprite, fireflower_harvest_sprite, Aura::fire, 5, 0, 20.0f, "Fire Flower", "Gives off fire aura." );
	corpseeater_plant = new PlantType( { fireflower_1_mesh, fireflower_2_mesh, fireflower_3_mesh }, corpseeater_seed_sprite, corpseeater_harvest_sprite, Aura::none, 5, 50, 40.0f, "Detritus Dahlia", "Feeds off a neighboring dead plant." );

	plant_mesh_buffer = ret;

	return ret;
} );

Load< GLuint > plant_meshes_for_firstpass_program( LoadTagDefault, [](){
	return new GLuint( plant_meshes->make_vao_for_program( firstpass_program->program ) );
} );

TileGrid setup_grid_for_scene( Scene& scene, int plant_grid_x, int plant_grid_y )
{

	TileGrid grid;
	// Make the tile grid
	{
		grid.tiles = new GroundTile * [plant_grid_x];
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			grid.tiles[x] = new GroundTile[plant_grid_y];
		}
	}

	grid.size_x = plant_grid_x;
	grid.size_y = plant_grid_y;

	//Populate the tile grid (default is sea)
	{
		Scene::Drawable::Pipeline default_info;
		default_info = firstpass_program_pipeline;
		default_info.vao = *plant_meshes_for_firstpass_program;
		default_info.start = 0;
		default_info.count = 0;
		// set default uniforms
		GLint PROPERTIES_vec3_loc = firstpass_program->PROPERTIES_vec3;
		default_info.set_uniforms = [PROPERTIES_vec3_loc](){
			glUniform3f(PROPERTIES_vec3_loc, 1.0f, 0.0f, 0.0f);
		};

		glm::vec3 tile_center_pos = glm::vec3( ( (float)plant_grid_x - 1 ) * plant_grid_tile_size.x / 2.0f, ( (float)plant_grid_y - 1 ) * plant_grid_tile_size.y / 2.0f, 0.0f );

		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				// Set coordinates
				grid.tiles[x][y].grid_x = x;
				grid.tiles[x][y].grid_y = y;

				// Set up tile drawable and initial pipline for each tile
				scene.transforms.emplace_back();
				Scene::Transform* tile_transform = &scene.transforms.back();
				tile_transform->position = glm::vec3( plant_grid_tile_size.x * x, plant_grid_tile_size.y * y, 0.0f ) - tile_center_pos;
				scene.drawables.emplace_back( tile_transform );
				Scene::Drawable* tile = &scene.drawables.back();
				tile->pipeline = default_info;
				grid.tiles[x][y].tile_drawable = tile;

				// Set up plant drawable and initial pipline for each plant (empty)
				scene.transforms.emplace_back();
				Scene::Transform* plant_transform = &scene.transforms.back();
				plant_transform->position = glm::vec3( plant_grid_tile_size.x * x, plant_grid_tile_size.y * y, 0.0f ) - tile_center_pos;
				scene.drawables.emplace_back( plant_transform );
				Scene::Drawable* plant = &scene.drawables.back();
				plant->pipeline = default_info;
				grid.tiles[x][y].plant_drawable = plant;

				// Set default type for the tile
				grid.tiles[x][y].change_tile_type( sea_tile );

			}
		}
	}

	return grid;
}

void GroundTile::change_tile_type( const GroundTileType* tile_type_in )
{
	if( tile_type_in )
	{
		tile_type = tile_type_in;
		tile_drawable->pipeline.start = tile_type->get_mesh()->start;
		tile_drawable->pipeline.count = tile_type->get_mesh()->count;
		if( tile_type->get_can_plant() ) {
			GLint PROPERTIES_vec3_loc = firstpass_program->PROPERTIES_vec3;
			tile_drawable->pipeline.set_uniforms = [this, PROPERTIES_vec3_loc](){
				glUniform3f(PROPERTIES_vec3_loc, 1.0f, moisture, fertility);
			};
		}
	}
	else
	{
		printf( "ERROR Passed in null tile type! \n" );
	}
}

void GroundTile::update( float elapsed, Scene::Transform* camera_transform, const TileGrid& grid )
{
	// update plant state
	if( plant_type )
	{
		float grow_power = elapsed * std::sqrtf(plant_health);

		if( !is_plant_dead() )
		{
			if( plant_type == test_plant )
			{
				current_grow_time += grow_power + elapsed * std::sqrtf(moisture * fertility);
			}
			else if( plant_type == friend_plant )
			{
				int neighbor = 0;
				for( int x = -1; x <= 1; x += 2 )
				{
					if( grid_x + x >= 0 && grid_x + x < grid.size_x )
					{
						GroundTile& tile = grid.tiles[grid_x + x][grid_y];
						const PlantType* plant = tile.plant_type;
						if( plant && !tile.is_plant_dead() )
						{
							neighbor++;
							//Boost the neighbor
							tile.current_grow_time += elapsed * 0.4f;
						}
					}
				}
				for( int y = -1; y <= 1; y += 2 )
				{
					if( grid_y + y >= 0 && grid_y + y < grid.size_y )
					{
						GroundTile& tile = grid.tiles[grid_x][grid_y + y];
						const PlantType* plant = tile.plant_type;
						if( plant && !tile.is_plant_dead() )
						{
							neighbor++;
							//Boost the neighbor
							tile.current_grow_time += elapsed * 0.4f;
						}
					}
				}

				if( neighbor >= 2 )
				{
					current_grow_time += grow_power + elapsed * std::sqrtf(moisture * fertility);
				}
				else
				{
					plant_health -= elapsed * ( plant_health_restore_rate + 0.1f );
				}
			}
			else if( plant_type == vampire_plant )
			{
				std::vector<GroundTile*> victims;

				for( int i = 0; i < 4; ++i )
				{
					int x = i < 2 ? 2 * i - 1 : 0;
					int y = i < 2 ? 0 : 2 * ( i - 2 ) - 1;
					if( grid_x + x >= 0 && grid_x + x < grid.size_x && grid_y + y >= 0 && grid_y + y < grid.size_y )
					{
						GroundTile& tile = grid.tiles[grid_x + x][grid_y + y];
						const PlantType* plant = tile.plant_type;
						if( plant && !tile.is_plant_dead() )
						{
							victims.push_back( &tile );
						}
					}
				}

				if( victims.size() > 0 )
				{
					victims[rand() % victims.size()]->plant_health -= elapsed * ( 2*plant_health_restore_rate + 0.2f );
					current_grow_time += grow_power + elapsed * std::sqrtf(moisture * fertility);
				}
				else
				{
					plant_health -= elapsed * ( plant_health_restore_rate + 0.1f );
				}
			}
			else if( plant_type == corpseeater_plant )
			{
				int dead_plants = 0;

				for( int i = 0; i < 4; ++i )
				{
					int x = i < 2 ? 2 * i - 1 : 0;
					int y = i < 2 ? 0 : 2 * ( i - 2 ) - 1;
					if( grid_x + x >= 0 && grid_x + x < grid.size_x && grid_y + y >= 0 && grid_y + y < grid.size_y )
					{
						GroundTile& tile = grid.tiles[grid_x + x][grid_y + y];
						const PlantType* plant = tile.plant_type;
						if( plant && tile.is_plant_dead() )
						{
							dead_plants++;
						}
					}
				}

				if( dead_plants > 0 )
				{
					current_grow_time += grow_power + std::sqrtf(moisture * fertility);
				}
				else
				{
					plant_health -= elapsed * ( plant_health_restore_rate + 0.1f );
				}
			}
			else if( plant_type == fireflower_plant )
			{
				current_grow_time += grow_power + elapsed * std::sqrtf(fertility);
			}
			else if( plant_type == cactus_plant )
			{
				if( fire_aura_effect > 0.1f && aqua_aura_effect <= 0.0f )
				{
					current_grow_time += grow_power * fire_aura_effect + elapsed * std::sqrtf(fertility);
				}
				else
				{
					plant_health -= elapsed * ( plant_health_restore_rate + 0.1f );
				}
			}

			if( plant_health < 1.0f ) plant_health = std::min( 1.0f, plant_health + elapsed * plant_health_restore_rate );
		}

		float target_time = plant_type->get_growth_time();
		if( current_grow_time > target_time ) current_grow_time = target_time;
		update_plant_visuals( current_grow_time / target_time );
	}

	// apply aura effect onto neighbors (by putting into pending update)
	if( plant_type && ( plant_type->get_aura_type() != Aura::none ) && !is_plant_dead() )
	{
		auto try_apply_aura = [elapsed]( GroundTile& target, Aura::Type aura_type ) {
			if( target.tile_type->get_can_plant() ) {
				switch( aura_type ) {
				case Aura::fire:
					target.pending_update.fire_aura_effect += 0.2f * elapsed;
					break;
				case Aura::aqua:
					target.pending_update.aqua_aura_effect += 0.2f * elapsed;
					break;
				default:
					std::cerr << "non-exhaustive matching of aura type??";
					break;
				}
			}
		};
		// get a list of neighbors
		std::vector< GroundTile* > neighbors = {};
		for( int x = -1; x <= 1; x += 2 ) {
			if( grid_x + x >= 0 && grid_x + x < grid.size_x ) {
				GroundTile& tile = grid.tiles[grid_x + x][grid_y];
				neighbors.push_back( &tile );
			}
		}
		for( int y = -1; y <= 1; y += 2 ) {
			if( grid_y + y >= 0 && grid_y + y < grid.size_y ) {
				GroundTile& tile = grid.tiles[grid_x][grid_y + y];
				neighbors.push_back( &tile );
			}
		}
		// apply effect
		for( auto tile_ptr : neighbors ) {
			assert( tile_ptr );
			try_apply_aura( *tile_ptr, plant_type->get_aura_type() );
		}
	}

	// update tile state
	moisture -= moisture_dry_rate * elapsed;
	if( plant_type && !is_plant_dead() ) fertility -= fertility_consume_rate * elapsed;
	fire_aura_effect = std::max( 0.0f, fire_aura_effect - 0.1f * elapsed );
	aqua_aura_effect = std::max( 0.0f, aqua_aura_effect - 0.1f * elapsed );
}

void GroundTile::update_plant_visuals( float percent_grown )
{
	if( plant_type )
	{
		plant_drawable->transform->scale = glm::mix( glm::vec3( 0.5f, 0.5f, 0.2f ), glm::vec3( 1.0f, 1.0f, 1.0f ), plant_type->get_stage_percent( percent_grown ) );
		const Mesh* plant_mesh = is_plant_dead() ? dead_plant_mesh : plant_type->get_mesh( percent_grown );
		plant_drawable->pipeline.start = plant_mesh->start;
		plant_drawable->pipeline.count = plant_mesh->count;
		// set health uniform. TODO: move this to somewhere that gets called less often
		GLint PROPERTIES_vec3_loc = firstpass_program->PROPERTIES_vec3;
		plant_drawable->pipeline.set_uniforms = [this, PROPERTIES_vec3_loc](){
			glUniform3f(PROPERTIES_vec3_loc, plant_health, 0.0f, 0.0f);
		};
	}
}

void GroundTile::apply_pending_update(float elapsed)
{
	// move update from pending_update
	fire_aura_effect = std::min( 1.0f, fire_aura_effect + pending_update.fire_aura_effect );
	aqua_aura_effect = std::min( 1.0f, aqua_aura_effect + pending_update.aqua_aura_effect );
	pending_update.fire_aura_effect = 0.0f;
	pending_update.aqua_aura_effect = 0.0f;

	//.. and continue updating what's left
	moisture += aqua_aura_effect * elapsed * 0.25f;
	moisture -= fire_aura_effect * elapsed * 0.25f;

	moisture = std::max( 0.0f, moisture );
	moisture = std::min( 1.0f, moisture );
	fertility = std::max( 0.0f, fertility );
	fertility = std::min( 1.0f, fertility );
	
}

void GroundTile::update_aura_visuals( float elapsed, Scene::Transform* camera_transform )
{ // TODO: always have aura created but only update when there is effect?
	// create corresponding aura if not already exist
	if( fire_aura_effect > 0 && (!fire_aura) ) {
		fire_aura = new Aura( tile_drawable->transform->position, Aura::fire );
	}
	if( aqua_aura_effect > 0 && (!aqua_aura) ) {
		aqua_aura = new Aura( tile_drawable->transform->position, Aura::fire );
	}
	// or delete if no longer has aura effect	
	if( fire_aura_effect == 0 && fire_aura ) {
		delete fire_aura;
		fire_aura = nullptr;
	}
	if( aqua_aura_effect == 0 && aqua_aura ) {
		delete aqua_aura;
		aqua_aura = nullptr;
	}
	// update aura accordingly
	if( fire_aura ) fire_aura->update( 
			int(floor(fire_aura_effect * fire_aura->max_strength)), // strength
			elapsed, camera_transform );
	if( aqua_aura ) aqua_aura->update( 
			int(floor(aqua_aura_effect * aqua_aura->max_strength)), // strength
			elapsed, camera_transform );
}

bool GroundTile::try_add_plant( const PlantType* plant_type_in )
{
	// If we can plant on the tile and there is no plant already there, add a plant
	if( tile_type->get_can_plant() && !plant_type )
	{
		plant_type = plant_type_in;
		plant_drawable->pipeline.start = plant_type->get_mesh( 0.0f )->start;
		plant_drawable->pipeline.count = plant_type->get_mesh( 0.0f )->count;

		current_grow_time = 0.0f;
		plant_health = 1.0f;
		update_plant_visuals( 0.0f );
		return true;
	}
	return false;
}

bool GroundTile::try_remove_plant()
{
	// If there is a plant on tile, kick it out and hide the drawable
	if( plant_type )
	{
		plant_type = nullptr;
		plant_drawable->pipeline.start = 0;
		plant_drawable->pipeline.count = 0;
		return true;
	}
	return false;
}

bool GroundTile::is_tile_harvestable()
{
	return plant_type && current_grow_time >= plant_type->get_growth_time() && !is_plant_dead();
}

bool GroundTile::is_plant_dead()
{
	return plant_type && plant_health <= 0.0f;
}

bool GroundTile::can_be_cleared(const TileGrid& grid ) const
{
	bool has_cleared_neighbor = false;
	for( int i = 0; i < 4; ++i )
	{
		int x_i = i < 2 ? 2*i - 1 : 0;
		int y_i = i < 2 ? 0 : 2 * ( i - 2 ) - 1;
		if( grid.is_in_grid( grid_x + x_i, grid_y + y_i ) && grid.tiles[grid_x + x_i][grid_y + y_i].is_cleared() )
		{
			has_cleared_neighbor = true;
			break;
		}
	}

	return has_cleared_neighbor && tile_type->get_clear_cost() > 0;
}

bool GroundTile::try_clear_tile()
{
	change_tile_type( ground_tile );
	return true;
}

bool GroundTile::is_cleared() const
{
	return tile_type == ground_tile;
}

bool TileGrid::is_in_grid( int x, int y ) const
{
	return x >= 0 && y >= 0 && x < size_x && y < size_y;
}

void PlantType::make_buttons( glm::vec2 screen_size, const PlantType** selectedPlant, Tool* current_tool, Button** seed_btn, Button** harvest_btn ) const {
	assert( selectedPlant );
	assert( seed_btn );
	assert( harvest_btn );
	assert( seed_sprite );
	assert( harvest_sprite );
	*seed_btn = new Button (
		screen_size, Button::tl, glm::vec2(100, 100), // position
		glm::vec2(64, 64), // size
		seed_sprite, // sprite
		glm::vec2(32, 32), // sprite anchor
		0.3f, // sprite scale
		Button::show_text, // hover behavior
		name, // text
		glm::vec2(0, -20), // text anchor
		0.4f, // text scale
		[this, selectedPlant, current_tool]() {
			*selectedPlant = this;
			*current_tool = seed;
		}, true);
	
	*harvest_btn = new Button (
		screen_size, Button::tl, glm::vec2(100, 200), // position
		glm::vec2(64, 64), // size
		harvest_sprite, // sprite
		glm::vec2(32, 32), // sprite anchor
		0.3f, // sprite scale
		Button::show_text, // hover behavior
		name, // text
		glm::vec2(0, -20), // text anchor
		0.4f, // text scale
		[]() { // empty for now
		}, true);
}

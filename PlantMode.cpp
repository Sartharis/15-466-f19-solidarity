#include "PlantMode.hpp"

#include "FirstpassProgram.hpp"
#include "PostprocessingProgram.hpp"
#include "ColorTextureProgram.hpp"
#include "Load.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"
#include "collide.hpp"
#include "DrawSprites.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>
#include <unordered_map>

PlantType const* test_plant = nullptr;
GroundTileType const* sea_tile = nullptr;
GroundTileType const* ground_tile = nullptr;
GroundTileType const* obstacle_tile = nullptr;

Mesh const* sea_tile_mesh = nullptr;
Mesh const* ground_tile_mesh = nullptr;
Mesh const* plant_mesh = nullptr;
Mesh const* obstacle_tile_mesh = nullptr;
Mesh const* billboard = nullptr;

Sprite const *aura = nullptr;

Load< SpriteAtlas > font_atlas( LoadTagDefault, []() -> SpriteAtlas const* {
	return new SpriteAtlas( data_path( "trade-font" ) );
} );

Load< SpriteAtlas> sprite_atlas( LoadTagDefault, []() -> SpriteAtlas const* {
	auto ret = new SpriteAtlas( data_path( "solidarity" ) );
	std::cout << "----2D sprites loaded:" << std::endl;
	for (auto p : ret->sprites) {
		std::cout << p.first << std::endl;
	}
	aura = &ret->lookup( "blurredDot" );
	return ret;
} );

Load< MeshBuffer > plant_meshes(LoadTagDefault, [](){
	auto ret = new MeshBuffer(data_path("solidarity.pnct"));
	std::cout << "----meshes loaded:" << std::endl;
	for (auto p : ret->meshes) {
		std::cout << p.first << std::endl;
	}
	sea_tile_mesh = &ret->lookup("unoccupied");
	ground_tile_mesh = &ret->lookup("soil");
	plant_mesh = &ret->lookup( "leaf2" );
	obstacle_tile_mesh = &ret->lookup("path");
	billboard = &ret->lookup("unit_sq");
	return ret;
});

Load< GLuint > plant_meshes_for_firstpass_program(LoadTagDefault, [](){
	return new GLuint(plant_meshes->make_vao_for_program(firstpass_program->program));
});

void GroundTile::change_tile_type( const GroundTileType* tile_type_in )
{
	tile_type = tile_type_in;
	tile_drawable->pipeline.start = tile_type->get_mesh()->start;
	tile_drawable->pipeline.count = tile_type->get_mesh()->count;
}

void GroundTile::update( float elapsed, glm::vec3 camera_position )
{
	if( plant_type )
	{
		float target_time = plant_type->get_growth_time();
		current_grow_time += elapsed;
		if( current_grow_time > target_time ) current_grow_time = target_time;
		update_plant_visuals( current_grow_time / target_time );
	}
	if( aura )
	{
		aura->update(elapsed, camera_position);
	}
}

void GroundTile::update_plant_visuals( float percent_grown )
{
	//TEMP!!!!!!
	plant_drawable->transform->position.z = glm::mix( start_height, end_height, percent_grown );
}

bool GroundTile::try_add_plant( const PlantType* plant_type_in )
{
	// If we can plant on the tile and there is no plant already there, add a plant
	if( tile_type->get_can_plant() && !plant_type )
	{
		plant_type = plant_type_in;
		plant_drawable->pipeline.start = plant_type->get_mesh()->start;
		plant_drawable->pipeline.count = plant_type->get_mesh()->count;

		current_grow_time = 0.0f;
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
	return plant_type && current_grow_time >= plant_type->get_growth_time();
}

Aura::Aura(SpriteAtlas const &_atlas) : atlas(_atlas) {
	dots = std::vector<Dot>(1);

	{ // opengl related
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glVertexAttribPointer(
			color_texture_program->Position_vec4,
			3, // size
			GL_FLOAT, // type
			GL_FALSE, // normalized
			sizeof(Aura::Vertex), // stride size
			(GLbyte*) 0 + offsetof(Aura::Vertex, position) // offset
		);
		glEnableVertexAttribArray(color_texture_program->Position_vec4);

		glVertexAttribPointer(
			color_texture_program->TexCoord_vec2, //attribute
			2, //size
			GL_FLOAT, //type
			GL_FALSE, //normalized
			sizeof(Aura::Vertex), // stride size
			(GLbyte *) 0 + offsetof(Aura::Vertex, tex_coord) //offset
		);
		glEnableVertexAttribArray(color_texture_program->TexCoord_vec2);

		glVertexAttribPointer(
			color_texture_program->Color_vec4, //attribute
			4, //size
			GL_UNSIGNED_BYTE, //type
			GL_TRUE, //normalized
			sizeof(Aura::Vertex), //stride
			(GLbyte *) 0 + offsetof(Aura::Vertex, color) //offset
		);
		glEnableVertexAttribArray(color_texture_program->Color_vec4);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
}

void Aura::update(float elapsed, glm::vec3 camera_position) {
	for(int i=0; i<dots.size(); i++) {
		// TODO: update position... then:
		glm::vec3 c = dots[i].position; // center of dot
		float r = dots[i].radius;
		glm::mat3 rotation = glm::mat3(glm::lookAt(
			dots[i].position, camera_position, glm::vec3(0, 0, 1)
		));
		Vertex tl = Vertex(rotation * glm::vec3(-0.5, 0.5, 0) * r + c, aura->min_px);
		Vertex tr = Vertex(rotation * glm::vec3(0.5, 0.5, 0) * r + c, glm::vec2(aura->max_px.x, aura->min_px.y));
		Vertex bl = Vertex(rotation * glm::vec3(-0.5, -0.5, 0) * r + c, glm::vec2(aura->min_px.x, aura->max_px.y));
		Vertex br = Vertex(rotation * glm::vec3(0.5, -0.5, 0) * r + c, aura->max_px);
		// make the square and append it to vbo
		std::vector<Vertex> billboard_this = { tl, bl, br, tl, br, tr };	
		dots_vbo.insert(dots_vbo.end(), billboard_this.begin(), billboard_this.end());
	}
}

void Aura::draw() {
	// config opengl
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	// draw the dots w ColorTextureProgram
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, dots_vbo.size() * sizeof(Vertex), dots_vbo.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(color_texture_program->program);
	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas.tex);

	glDrawArrays(GL_TRIANGLES, 0, GLsizei(dots_vbo.size() * sizeof(Vertex)));

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}


PlantMode::PlantMode() 
{
	{
		sea_tile = new GroundTileType( false, sea_tile_mesh );
		ground_tile = new GroundTileType( true, ground_tile_mesh );
		obstacle_tile = new GroundTileType( false, obstacle_tile_mesh );
		test_plant = new PlantType( plant_mesh, 5, 7, 5.0f, "Fern", "Cheap plant. Grows anywhere." );
		selectedPlant = test_plant;
	}

	// Make the tile grid
	{
		grid = new GroundTile*[plant_grid_x];
		for( int32_t x = 0; x < plant_grid_x; ++x ) 
		{
			grid[x] = new GroundTile[plant_grid_y];
		}
	}

	//Populate the tile grid (default is sea)
	{
		Scene::Drawable::Pipeline default_info;
		default_info = firstpass_program_pipeline;
		default_info.vao = *plant_meshes_for_firstpass_program;
		default_info.start = 0;
		default_info.count = 0;

		glm::vec3 tile_center_pos = glm::vec3( ( (float)plant_grid_x - 1 ) * plant_grid_tile_size.x / 2.0f, ( (float)plant_grid_y - 1 ) * plant_grid_tile_size.y / 2.0f, 0.0f );

		for( int32_t x = 0; x < plant_grid_x; ++x ) 
		{
			for( int32_t y = 0; y < plant_grid_y; ++y ) 
			{
				// Set coordinates
				grid[x][y].grid_x = x;
				grid[x][y].grid_y = y;

				// Set up tile drawable and initial pipline for each tile
				scene.transforms.emplace_back();
				Scene::Transform* tile_transform = &scene.transforms.back();
				tile_transform->position = glm::vec3( plant_grid_tile_size.x * x, plant_grid_tile_size.y * y, 0.0f ) - tile_center_pos;
				scene.drawables.emplace_back( tile_transform );
				Scene::Drawable* tile = &scene.drawables.back();
				tile->pipeline = default_info;
				grid[x][y].tile_drawable = tile;

				// Set up plant drawable and initial pipline for each plant (empty)
				scene.transforms.emplace_back();
				Scene::Transform* plant_transform = &scene.transforms.back();
				plant_transform->position = glm::vec3( plant_grid_tile_size.x * x, plant_grid_tile_size.y * y, 0.0f ) - tile_center_pos;
				scene.drawables.emplace_back( plant_transform );
				Scene::Drawable* plant = &scene.drawables.back();
				plant->pipeline = default_info;
				grid[x][y].plant_drawable = plant;

				// Set default type for the tile
				grid[x][y].change_tile_type(sea_tile);

			}
		}
	}


	// Create a lil center island
	{
		for( int32_t x = 7; x < 13; ++x )
		{
			for( int32_t y = 7; y < 13; ++y )
			{
				grid[x][y].change_tile_type( obstacle_tile );
			}
		}

		for( int32_t x = 8; x < 12; ++x )
		{
			for( int32_t y = 8; y < 12; ++y )
			{
				grid[x][y].change_tile_type( ground_tile );
			}
		}
	}


	{ //make a camera:
		scene.transforms.emplace_back();
		Scene::Transform *transform = &scene.transforms.back();
		transform->position = glm::vec3(0.0f, 0.0f, 0.0f);
		transform->rotation = glm::quat_cast(glm::mat3(glm::lookAt(
			transform->position,
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		)));
		scene.cameras.emplace_back(transform);
		camera = &scene.cameras.back();
		camera->near = 0.01f;
		camera->fovy = glm::radians(45.0f);
	}

	{ // init the opengl stuff
		// ------ generate framebuffer for firstpass
		glGenFramebuffers(1, &firstpass_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, firstpass_fbo);
		// and its two color output layers
		glGenTextures(2, colorBuffers);
		for (GLuint i=0; i<2; i++) {
			glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
			glTexImage2D(
				// ended up disabling high resolution draw so the program runs at a reasonable framerate...
				GL_TEXTURE_2D, 0, GL_RGBA, 
				(GLint)(screen_size.x/postprocessing_program->pixel_size), 
				(GLint)(screen_size.y/postprocessing_program->pixel_size), 
				0, GL_RGBA, GL_FLOAT, NULL    
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, colorBuffers[i], 0    
			);
		}
		// setup associated depth buffer
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)screen_size.x, (GLsizei)screen_size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

		glDrawBuffers(2, color_attachments);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

		// ------ set up 2nd pass pipeline
		glGenVertexArrays(1, &trivial_vao);
		glBindVertexArray(trivial_vao);

		glGenBuffers(1, &trivial_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, trivial_vbo);
		glBufferData(
			GL_ARRAY_BUFFER, 
			trivial_vector.size() * sizeof(float),
			trivial_vector.data(),
			GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		GL_ERRORS();

		// ------ ping pong framebuffers for gaussian blur (use this for aura effect later)
		glGenFramebuffers(2, pingpong_fbo);
		glGenTextures(2, pingpongBuffers);
		for (unsigned int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, pingpong_fbo[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i]);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)screen_size.x, (GLsizei)screen_size.y, 0, GL_RGBA, GL_FLOAT, NULL
			); // w&h of drawable size
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i], 0
			);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}

PlantMode::~PlantMode() {
}

void PlantMode::on_click( int x, int y )
{
	GroundTile* collided_tile = get_tile_under_mouse( x, y );

	if( collided_tile )
	{
		if( collided_tile->plant_type )
		{
			if( collided_tile->is_tile_harvestable() )
			{
				int gain = collided_tile->plant_type->get_harvest_gain();
				if( collided_tile->try_remove_plant() )
				{
					energy += gain;
				}
			}
		}
		else
		{
			if( collided_tile->tile_type == obstacle_tile )
			{
				collided_tile->change_tile_type(ground_tile);
			}
			else if(selectedPlant && energy >= selectedPlant->get_cost())
			{
				if( collided_tile->try_add_plant( selectedPlant ) )
				{
					energy -= selectedPlant->get_cost();
				}
			}
		}
	}
}

GroundTile* PlantMode::get_tile_under_mouse( int x, int y )
{
	//Get ray from camera to mouse in world space
	GLint dim_viewport[4];
	glGetIntegerv( GL_VIEWPORT, dim_viewport );
	int width = dim_viewport[2];
	int height = dim_viewport[3];

	glm::vec3 ray_nds = glm::vec3( 2.0f * x / width - 1.0f, 1.0f - ( 2.0f * y ) / height, 1.0f );
	glm::vec4 ray_clip = glm::vec4( ray_nds.x, ray_nds.y, -1.0f, 1.0f );
	glm::vec4 ray_cam = glm::inverse( camera->make_projection() ) * ray_clip;
	ray_cam = glm::vec4( ray_cam.x, ray_cam.y, -1.0f, 0.0f );
	glm::vec4 ray_wort = glm::inverse( camera->transform->make_world_to_local() ) * ray_cam;
	glm::vec3 ray_wor = glm::vec3( ray_wort.x, ray_wort.y, ray_wort.z );
	ray_wor = glm::normalize( ray_wor );


	float col_check_dist = 1000.0f;
	glm::vec3 from_camera_start = camera->transform->position;
	glm::vec3 from_camera_dir = ray_wor;//camera->transform->rotation * glm::vec3( 0.0f, 0.0f, -1.0f );

	// Check collision against each tile
	GroundTile* collided_tile = nullptr;
	for( int32_t x = 0; x < plant_grid_x; ++x )
	{
		for( int32_t y = 0; y < plant_grid_y; ++y )
		{
			// For now do a small sphere sweep against each triangle (TODO: optimize to line vs box collision if this is really bad)
			float sphere_radius = 0.0001f;
			glm::vec3 sphere_sweep_from = from_camera_start;
			glm::vec3 sphere_sweep_to = from_camera_start + col_check_dist * from_camera_dir;

			glm::vec3 sphere_sweep_min = glm::min( sphere_sweep_from, sphere_sweep_to ) - glm::vec3( sphere_radius );
			glm::vec3 sphere_sweep_max = glm::max( sphere_sweep_from, sphere_sweep_to ) + glm::vec3( sphere_radius );
			(void)sphere_sweep_min;
			(void)sphere_sweep_max;

			float collision_t = 1.0f;
			glm::vec3 collision_at = glm::vec3( 0.0f );
			glm::vec3 collision_out = glm::vec3( 0.0f );

			glm::mat4x3 collider_to_world = grid[x][y].tile_drawable->transform->make_local_to_world();
			const Mesh& collider_mesh = *( grid[x][y].tile_type->get_mesh() );

			assert( collider_mesh.type == GL_TRIANGLES ); //only have code for TRIANGLES not other primitive types
			for( GLuint v = 0; v + 2 < collider_mesh.count; v += 3 )
			{

				//get vertex positions from associated positions buffer:
				glm::vec3 a = collider_to_world * glm::vec4( plant_meshes->positions[collider_mesh.start + v + 0], 1.0f );
				glm::vec3 b = collider_to_world * glm::vec4( plant_meshes->positions[collider_mesh.start + v + 1], 1.0f );
				glm::vec3 c = collider_to_world * glm::vec4( plant_meshes->positions[collider_mesh.start + v + 2], 1.0f );
				//check triangle:
				bool did_collide = collide_swept_sphere_vs_triangle(
					sphere_sweep_from, sphere_sweep_to, sphere_radius,
					a, b, c,
					&collision_t, &collision_at, &collision_out );

				if( did_collide )
				{
					collided_tile = &grid[x][y];
				}
			}
		}
	}
	
	return collided_tile;
}

bool PlantMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}

	if( evt.type == SDL_MOUSEBUTTONDOWN )
	{
		if( evt.button.button == SDL_BUTTON_LEFT )
		{
			on_click( evt.motion.x, evt.motion.y );
		}
		return false;
	}

	return false;
}

void PlantMode::update(float elapsed) 
{
	//camera_azimuth += elapsed;

	// Update Camera Position
	{
		float ce = std::cos( camera_elevation );
		float se = std::sin( camera_elevation );
		float ca = std::cos( camera_azimuth );
		float sa = std::sin( camera_azimuth );
		camera->transform->position = camera_radius * glm::vec3( ce * ca, ce * sa, se );
		camera->transform->rotation =
			glm::quat_cast( glm::transpose( glm::mat3( glm::lookAt(
			camera->transform->position,
			glm::vec3( 0.0f, 0.0f, 0.0f ),
			glm::vec3( 0.0f, 0.0f, 1.0f )
			) ) ) );
	}

	// update tiles
	{
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				grid[x][y].update( elapsed, camera->transform->position );
			}
		}
	}

	// Query for hovered tile
	int x, y;
	const Uint32 state = SDL_GetMouseState( &x, &y );
	(void)state;
	if( true )
	{
		GroundTile* hovered_tile = get_tile_under_mouse( x, y );
		if( hovered_tile && hovered_tile->plant_type)
		{
			if( hovered_tile->is_tile_harvestable() )
			{
				action_description = "Harvest +" + std::to_string( hovered_tile->plant_type->get_harvest_gain());
			}
			else
			{
				action_description = "Growing "; //+ std::to_string(hovered_tile->current_grow_time / hovered_tile->plant_type->get_growth_time());
			}
		}
		else if ( hovered_tile && selectedPlant && hovered_tile->tile_type->get_can_plant() )
		{
			action_description = "Plant -" + std::to_string(selectedPlant->get_cost());
		}
		else
		{
			action_description = "";
		}
	}
}

void PlantMode::draw(glm::uvec2 const &drawable_size) {
	//Draw scene:
	camera->aspect = drawable_size.x / float(drawable_size.y);

	//---- first pass ----
	glBindFramebuffer(GL_FRAMEBUFFER, firstpass_fbo);
	glViewport(0, 0, 
		(GLsizei)(screen_size.x / postprocessing_program->pixel_size),
		(GLsizei)(screen_size.y / postprocessing_program->pixel_size));
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//-- set up basic OpenGL state --
	// depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// blend
	glDisable(GL_BLEND);
	// draw the scene
	scene.draw(*camera);

	//---- postprocessing pass ----
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(postprocessing_program->program);
	glBindVertexArray(trivial_vao);
	glBindBuffer(GL_ARRAY_BUFFER, trivial_vbo);
	glActiveTexture(GL_TEXTURE0);
	glViewport(0, 0, (GLsizei)screen_size.x, (GLsizei)screen_size.y);
	// set uniform so the shader performs copy to screen directly
	glUniform1i(postprocessing_program->TASK_int, 3);
	// set uniform for texture offset
	glUniform2f(postprocessing_program->TEX_OFFSET_vec2, 
		postprocessing_program->pixel_size / screen_size.x, 
		postprocessing_program->pixel_size / screen_size.y);
	// bind input
	glUniform1i(postprocessing_program->TEX1_tex, 0);
	glUniform1i(postprocessing_program->TEX2_tex, 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);

	// draw
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// unbind things
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	GL_ERRORS();

	// TEXT
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_DEPTH_TEST );

	{ //draw all the text
		DrawSprites draw( *font_atlas, glm::vec2( -1.0f, -1.0f ), glm::vec2( 1.0f, 1.0f ), drawable_size, DrawSprites::AlignSloppy );
		draw.draw_text( selectedPlant->get_name() + " (" + std::to_string(selectedPlant->get_cost()) +") :" + selectedPlant->get_description(), glm::vec2( -1.5f, 0.85f ), 0.006f);
		draw.draw_text( "Energy: " + std::to_string( energy ), glm::vec2( 0.7f, 0.85f ), 0.006f );
		draw.draw_text( action_description, glm::vec2( 0.7f, 0.75f ), 0.006f );
	}
}



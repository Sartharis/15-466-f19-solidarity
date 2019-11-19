#pragma once

#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "Load.hpp"
#include "Sound.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <functional>

struct UIElem {

	enum Action { mouseDown, mouseUp, mouseEnter, mouseLeave, none };

	UIElem(
			UIElem* _parent, // hierarchy
			glm::vec2 _anchor = glm::vec2(0, 0), // transform
			glm::vec2 _position = glm::vec2(0, 0),
			glm::vec2 _size = glm::vec2(0, 0),
			Sprite const* _sprite = nullptr, // content
			std::string _text = "",
			glm::vec2 _sprite_position = glm::vec2(0, 0),
			float _scale = 1.0f,
			bool _interactive = false, // optional
			bool _hidden = false,
			bool _default_sound = true
			) : sprite(_sprite),
					text(_text),
					sprite_position(_sprite_position),
					scale(_scale),
					anchor(_anchor),
					position(_position),
					size(_size),
					parent(_parent),
					interactive(_interactive),
					hidden(_hidden),
					default_sound(_default_sound) {
						if (parent) parent->add_child(this);
					}

	~UIElem();

	static bool z_index_comp_fn(UIElem* e1, UIElem* e2) {
		return e1->get_absolute_z_index() < e2->get_absolute_z_index();
	} 

	void draw(DrawSprites& draw_sprites, DrawSprites& draw_text);
	void draw_self(DrawSprites& draw_sprites, DrawSprites& draw_text); // draw this elem only (not its children)
	void gather(std::vector<UIElem*> &list);

	Action test_event(glm::vec2 mouse_pos, Action action);

	void set_position(glm::vec2 _position, glm::vec2 _anchor, glm::vec2 screen_size);
	void set_parent(UIElem* _parent); 
	void set_text(std::string _text){ text = _text; }
	void set_z_index(int _z_index){ z_index = _z_index; }
	void set_tint(glm::u8vec4 _tint){ tint = _tint; }

	glm::vec2 get_position(){ return position; }
	glm::vec2 get_anchor(){ return anchor; }
	Sprite const* get_sprite(){ return sprite; }
	std::string get_text(){ return text; }
	bool get_hidden(){ return hidden; }

	void update_absolute_position(glm::vec2 new_screen_size);
	void add_child(UIElem* child){ children.push_back(child); }
	void layout_children();

	void set_on_mouse_down(std::function<void()> fn){ on_mouse_down = fn; }
	void set_on_mouse_up(std::function<void()> fn){ on_mouse_up = fn; }
	void set_on_mouse_enter(std::function<void()> fn){ on_mouse_enter = fn; }
	void set_on_mouse_leave(std::function<void()> fn){ on_mouse_leave = fn; }
	void set_layout_children_fn(std::function<void()> fn){ layout_children_fn = fn; }
	void show(){ hidden = false; }
	void hide(){ hidden = true; }

	void print_name(){ std::cout << text << std::endl; }
	
	std::vector<UIElem*> children = std::vector<UIElem*>();

private:

	// content
	Sprite const* sprite = nullptr;
	std::string text = "";
	glm::vec2 sprite_position = glm::vec2(0,0);
	float scale = 1.0f;
	glm::u8vec4 tint = glm::vec4(255, 255, 255, 255);

	// transformation states
	glm::vec2 anchor = glm::vec2(0,0); // origin (rel to parent) from which position is specified
	glm::vec2 position = glm::vec2(0,0);
	glm::vec2 size = glm::vec2(0,0);
	// transformation states that get automatically updated on resize
	glm::vec2 absolute_position = glm::vec2(0,0);

	// hierarchy
	UIElem* parent = nullptr;
	int z_index = 0; // relative to parent. Low z-index elements get drawn first (show at bottom)

	// interaction behaviors
	std::function<void()> on_mouse_down = nullptr;
	std::function<void()> on_mouse_up = nullptr;
	std::function<void()> on_mouse_enter = nullptr;
	std::function<void()> on_mouse_leave = nullptr;
	std::function<void()> layout_children_fn = nullptr;
	
	// internal states
	bool interactive = false;
	bool hidden = false;
	bool default_sound = true;

	bool hovered = false;

	// helpers
	bool inside(glm::vec2 mouse_pos);
	bool get_hidden_from_hierarchy();
	int get_absolute_z_index();
	
};

extern Load< Sound::Sample > button_hover_sound;
extern Load< Sound::Sample > button_click_sound;

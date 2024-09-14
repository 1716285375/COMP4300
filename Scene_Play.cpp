#include <iostream>
#include <fstream>

#include "SFML//Window/Event.hpp"
//#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "Assets.h"
// #include "Physics.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"
#include "SFML/Graphics/RectangleShape.hpp"



Scene_Play::Scene_Play(GameEngine* gameEngine, const std::string& levelPath)
	: Scene(gameEngine), m_levelPath(levelPath) {
	init(levelPath);
}

void Scene_Play::init(const std::string& levelPath) {
	m_actionMap.clear();

	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");   // Toggle drawing (T)extures
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION"); // Toggle drawing (C)ollision Boxes
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");      // Toggle drawing (G)rid

	// TODO: Register all other gameplay Actions
	// registerAction(sf::Keyboard::W, "JUMP");
	registerAction(sf::Keyboard::Escape, "BACK");

	registerAction(sf::Keyboard::W, "UP");
	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::S, "DOWN");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Space, "JUMP");
	registerAction(sf::Keyboard::J, "SHOOT");

	m_gridText.setCharacterSize(12);
	m_gridText.setFont(m_game->assets().getFont("Mario"));
	// m_gridText.setFont(m_game->assets().getFont("Tech"));

	loadLevel(levelPath);
}

vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity) {
	// TODO: This function takes in a grid (x,y) position and an Entity
	//       Return a vec2 indicating where the CENTER position of the Entity should be
	//       You must use the Entity's Animation size to position it correctly
	//       The size of the grid width and height is stored in m_gridSize.x and m_gridSize.y
	//       The bottom-left corner of the Animation should aligh with the bottom left of the grid cell

	return vec2(0, 0);
}

void Scene_Play::loadLevel(const std::string& fileName) {
	// reset the entity manager every time we load a level
	m_entityManager = EntityManager();


	// TODO: read in the level file and add the appropriate entities
	//       use the PlayerConfig struct m_playerConfig to store player properties
	//       this struct is defined at the top of Scene_Play.h

	std::ifstream file(fileName);
	if (!file)
	{
		std::cerr << "Could not load " << fileName << " file\n";
		exit(-1);
	}

	std::string assetType;
	while (file >> assetType)
	{
		if (assetType == "Player")
		{
			file >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY
				>> m_playerConfig.SPEED >> m_playerConfig.MAX_SPEED
				>> m_playerConfig.JUMP >> m_playerConfig.GRAVITY
				>> m_playerConfig.WEAPON;

			//std::cout << m_playerConfig.X << std::endl;
		}
		else if (assetType == "Tile")
		{
			std::string name;
			float x, y;
			file >> name >> x >> y;

			bool loop = false;
			if (name == "Question")
				loop = true;

			//std::cout << name << " " << x << " " << y << std::endl;

			const Animation& animation = m_game->assets().getAnimation(name);
			//std::cout << animation.getName() << std::endl;
			sf::Sprite& sprite = const_cast<Animation&>(m_game->assets().getAnimation(name)).getSprite();
			// get collision size
			auto cx = sprite.getTexture()->getSize().x;
			auto cy = sprite.getTexture()->getSize().y;

			auto e = m_entityManager.addEntity("tile");

			e->addComponent<CTransform>(vec2(x * m_gridSize.x + m_gridSize.x / 2 , height() - y * m_gridSize.y - m_gridSize.y / 2));
			e->addComponent<CAnimation>(m_game->assets().getAnimation(name), false);
			e->addComponent<CBoundingBox>(vec2(cx, cy));

			e->getComponent<CAnimation>().has = true;
			e->getComponent<CBoundingBox>().has = true;
			e->getComponent<CTransform>().has = true;
			
		}
		else if (assetType == "Dec")
		{
			std::string name;
			int x, y;
			file >> name >> x >> y;
			const auto& animation = m_game->assets().getAnimation(name);

			auto e = m_entityManager.addEntity("Dec");

			e->addComponent<CTransform>(vec2(x * m_gridSize.x, height() - y * m_gridSize.y));
			e->addComponent<CAnimation>(animation, false);

			e->getComponent<CTransform>().has = true;
			e->getComponent<CAnimation>().has = true;
		}


	}

	// NOTE: all the code below is sample code which shows you how to
	//       set up and use entities with the new syntax, it should be removed

	spawnPlayer();

	// some sample entities
	auto brick = m_entityManager.addEntity("tile");
	// IMPORTANT: always add the CAnimation component first so that gridToMidPixel can compute correctly
	brick->addComponent<CAnimation>(m_game->assets().getAnimation("Brick"), true);
	brick->addComponent<CTransform>(vec2(96, 480));
	// NOTE: Your final code should position the entity with the grid x,y position read from the file:
	// brick->addComponent<CTransform>(gridToMidPixel(gridX, gridY, brick));

	if (brick->getComponent<CAnimation>().animation.getName() == "Brick") {
		//std::cout << "This could be a good way of identifying if a tile is a brick!\n";
	}

	auto block = m_entityManager.addEntity("tile");
	// block->addComponent<CAnimation>(m_game->assets().getAnimation("Block"), true);
	block->addComponent<CAnimation>(m_game->assets().getAnimation("Ground"), true);
	block->addComponent<CTransform>(vec2(224, 480));
	// add a bounding box, this will now show up if we press the 'C' key
	// block->addComponent<CBoundingBox>(m_game->assets().getAnimation("Block").getSize());
	block->addComponent<CBoundingBox>(m_game->assets().getAnimation("Ground").getSize());

	auto question = m_entityManager.addEntity("tile");
	question->addComponent<CAnimation>(m_game->assets().getAnimation("Ground"), true);
	question->addComponent<CTransform>(vec2(352, 480));

	// NOTE: THIS IS INCREDIBLY IMPORTANT PLEASE READ THIS EXAMPLE
	//       Components are now returned as references rather than pointers
	//       If you do not specify a reference variable type, it will COPY the component
	//       Here is an example:
	//
	//       This will COPY the transform into the variable 'transform1' - it is INCORRECT
	//       Any changes you make to transform1 will not be changed inside the entity
	//       auto transform1 = entity->get<CTransform>()
	//
	//       This will REFERENCE the transform with the variable 'transform2' - it is CORRECT
	//       Now any changes you make to transform2 will be changed inside the entity
	//       auto& transform2 = entity->get<CTransform>()
}

void Scene_Play::spawnPlayer() {
	// here is a sample player entity which you can use to construct other entities
	m_player = m_entityManager.addEntity("player");
	m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
	m_player->addComponent<CTransform>(vec2(m_playerConfig.X, height() - m_playerConfig.Y));
	m_player->addComponent<CBoundingBox>(vec2(m_playerConfig.CX / 2, m_playerConfig.CY));

	//std::cout << " pos: x: " << m_player->getComponent<CTransform>().pos.x << " "
	//	<< m_player->getComponent<CTransform>().pos.y << std::endl;

	// TODO: be sure to add the remaining components to the player

	m_player->addComponent<CInput>();
	m_player->addComponent<CGravity>(m_playerConfig.GRAVITY);
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity) {
	// TODO: this should spawn a bullet at the given entity, going in the direction the entity is facing
}

void Scene_Play::update() {
	m_entityManager.update();

	// TODO: implement pause functionality

	sMovement();
	sLifespan();
	sCollision();
	sAnimation();
	sRender();
	// m_currentFrame++;
}

void Scene_Play::sMovement() {
	// TODO: Implement player movement/jumping based on its CInput component
	// TODO: Implement gravity's effect on the player
	// TODO: Implement the maximum player speed in both X and Y directions
	// NOTE: Setting an entity's scale.x to -1/1 will make it face to the left/right



	for (auto& e : m_entityManager.getEntities())
	{
		
		if (!e->getComponent<CInput>().canJump)
			e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;

	}
}

void Scene_Play::sLifespan() {
	// TODO: Check lifespan of entities the have them, and destroy them if the go over
}

void Scene_Play::sCollision() {
	// REMEMBER: SFML's (0,0) position is in the TOP-LEFT corner
	//           This means jumping will have a negative y-component
	//           and gravity will have a positive y-component
	//           Also, something BELOW something else will hava a y value GREATER than it
	//           Also, something ABOVE something else will hava a y value LESS than it

	// TODO: Implement Physics::GetOverlap() function, use it inside this function

	// TODO: Implement bullet/tile collisions
	//       Destroy the tile if it has a Brick animation
	// TODO: Implement player/tile collisions and resolutions
	//       Update the CState component of the player to store whether
	//       it is currently on the ground or in the air. This will be
	//       used by the Animation system
	// TODO: Check to see if the player has fallen down a hole (y > height())
	// TODO: Don't let the player walk off the left side of the map
}

void Scene_Play::sDoAction(const Action& action) {
	std::cout << action.type() << " " << action.name() << std::endl;
	if (action.type() == "START")
	{
		std::cout << "this is start" << std::endl;
		if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
		else if (action.name() == "TOGGLE_COLLISION") { m_drawCollision = !m_drawCollision; }
		else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }
		else if (action.name() == "PAUSE") { setPaused(!m_paused); }
		else if (action.name() == "BACK")
		{
			//m_game->changeScene("MENU", m_game->getScene("MENU"));
			//auto menu_scene = std::make_shared<Scene_Menu>(m_game);
			//std::shared_ptr<Scene> scene = std::dynamic_pointer_cast<Scene>(menu_scene);
			auto menu = m_game->getScene("MENU");
			//menu->
			m_game->changeScene("MENU", menu);
		}
		else if (action.name() == "QUIT") { onEnd(); }
		else if (action.name() == "JUMP")
		{
			// m_player jump
			auto& e = m_player->getComponent<CTransform>();
			e.velocity = vec2(8, -8);
			m_player->getComponent<CInput>().canJump = false;
			auto& a = m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Air"), true);
			std::cout << "SPACE was Pressed: " << e.velocity.x << " "
				<< e.velocity.y << std::endl;

		}
		else if (action.name() == "STAND")
		{

		}
		else if (action.name() == "RUN")
		{

		}
		else if (action.name() == "SHOOT")
		{

		}
		else if (action.name() == "UP")
		{

		}
		else if (action.name() == "LEFT")
		{

		}
		else if (action.name() == "DOWN")
		{

		}
		else if (action.name() == "RIGHT")
		{

		}


	}

	if (action.type() == "END")
	{
		std::cout << "this is end" << std::endl;
		if (action.name() == "SHOOT")
		{
			std::cout << "SHOOT END" << std::endl;
		}
		else if (action.name() == "JUMP")
		{
			// m_player jump

			auto& e = m_player->getComponent<CTransform>();
			e.velocity = vec2(0, 0) - e.velocity;
			m_player->getComponent<CInput>().canJump = true;
			std::cout << "SPACE was Released: " << e.velocity.x << " "
				<< e.velocity.y << std::endl;
		}
	}

}

void Scene_Play::sAnimation() {
	// TODO: Complete the Animation class code first
	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CAnimation>())
		{

		}
	}

	auto& animation = m_player->getComponent<CAnimation>().animation;
	animation.update();
}

void Scene_Play::onEnd() {
	// TODO: when the scene ends, change back to the MENU scene
	// use m_game->changeScene(correct params);
	// m_game->changeScene( "MENU", std::make_shared<Scene_Menu>(m_game));
}

void Scene_Play::sRender() {
	// color the background darker, so you know that the game is paused
	if (!m_paused) {
		m_game->window().clear(sf::Color(100, 100, 255));
	}
	else {
		m_game->window().clear(sf::Color(50, 50, 150));
	}

	// set the viewport of the window to be centered on the player if it's far enough right
	auto& pPos = m_player->getComponent<CTransform>().pos;
	float windowCenterX = std::max(m_game->window().getSize().x / 2.0f, pPos.x);
	sf::View view = m_game->window().getView();
	//std::cout << "begin " << view.getSize().x << " " << view.getSize().y << std::endl;
	view.setCenter(windowCenterX, m_game->window().getSize().y - view.getCenter().y);
	//std::cout << "Center " << m_game->window().getSize().y << " " << view.getCenter().y << std::endl;
	m_game->window().setView(view);

	// draw all Entity textures / animations
	if (m_drawTextures) {
		for (const auto& e : m_entityManager.getEntities()) {
			auto& transform = e->getComponent<CTransform>();
			if (e->hasComponent<CAnimation>()) {
				auto& animation = e->getComponent<CAnimation>().animation;
				//animation.getSprite().setTextureRect(sf::IntRect(
				//	0, 0,
				//	animation.getSize().x, animation.getSize().y));
				//animation.getSprite().setRotation(transform.angle);
				////if (e->tag() == "player")
				////{
				////	auto vec = animation.getSize();
				////	animation.getSprite().setTextureRect(sf::IntRect(vec.x , 0, -vec.x, vec.y));
				////	//std::cout << -vec.x << " " << vec.y << std::endl;
				////}
				animation.getSprite().setPosition(
					transform.pos.x, transform.pos.y
				);
				//animation.getSprite().setScale(
				//	transform.scale.x, transform.scale.y
				//);
				m_game->window().draw(animation.getSprite());
			}
		}
	}

	// draw all Entity collision bounding boxes with a rectangle shape
	if (m_drawCollision) {
		for (const auto& e : m_entityManager.getEntities()) {
			if (e->hasComponent<CBoundingBox>()) {
				auto& box = e->getComponent<CBoundingBox>();
				auto& transform = e->getComponent<CTransform>();
				sf::RectangleShape rect;
				rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
				rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
				rect.setPosition(transform.pos.x, transform.pos.y);
				rect.setFillColor(sf::Color(0, 0, 0, 0));
				rect.setOutlineColor(sf::Color::White);
				rect.setOutlineThickness(1);
				m_game->window().draw(rect);
			}
		}
	}

	// draw the grid so that can easily debug
	if (m_drawGrid) {
		float leftX = m_game->window().getView().getCenter().x - width() / 2.0;
		float rightX = leftX + width() + m_gridSize.x;
		float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

		//std::cout << m_game->window().getView().getCenter().x << " " << width() << " " << leftX << " " << rightX << std::endl;

		for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
			drawLine(vec2(x, 0), vec2(x, height()));
		}

		for (float y = 0; y < height(); y += m_gridSize.y) {
			drawLine(vec2(leftX, height() - y), vec2(rightX, height() - y));

			for (float x = nextGridX; x < rightX; x += m_gridSize.x) {
				std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
				std::string yCell = std::to_string((int)y / (int)m_gridSize.y);
				m_gridText.setCharacterSize(4);
				m_gridText.setString("(" + xCell + "," + yCell + ")");
				m_gridText.setPosition(x + 3, height() - y - m_gridSize.y + 2);
				m_game->window().draw(m_gridText);
			}
		}
	}
}

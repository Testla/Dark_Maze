#include "StartScene.h"
#include "SelectLevelScene.h"
#include "Maze.h"
#pragma execution_character_set("utf-8")
USING_NS_CC;

Scene* StartScene::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = StartScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool StartScene::init() {
	if (!Layer::init())
	{
		return false;
	}

	winSize = Director::getInstance()->getWinSize();

	Sprite* background = Sprite::create("StartScene.png");
	background->setPosition(winSize / 2);
	addChild(background);

	auto item1 = MenuItemLabel::create(Label::createWithSystemFont("新游戏", "Microsoft Yahei", 30.0f), CC_CALLBACK_1(StartScene::start, this));
	auto item2 = MenuItemLabel::create(Label::createWithSystemFont("游戏说明", "Microsoft Yahei", 30.0f), CC_CALLBACK_0(StartScene::showInformation1, this));
	auto menu = Menu::create(item1, item2, NULL);
	menu->setPosition(winSize.width / 2, winSize.height / 2);
	menu->alignItemsVerticallyWithPadding(item1->getContentSize().height / 2);
	this->addChild(menu, 1);

	return true;
}

void StartScene::start(Ref* ref) {
	auto scene = SelectLevel::createScene();
	Director::getInstance()->replaceScene(scene);
}

void StartScene::showInformation1() {
	information = Sprite::create("information1.png");
	information->setPosition(winSize.width / 2, winSize.height / 2);
	addChild(information, 2);
	item = MenuItemLabel::create(Label::createWithSystemFont("Next", "Microsoft Yahei", 30.0f), CC_CALLBACK_0(StartScene::showInformation2, this));
	menu = Menu::create(item, NULL);
	menu->setPosition(winSize.width / 2, winSize.height / 10);
	menu->alignItemsVerticallyWithPadding(item->getContentSize().height / 2);
	this->addChild(menu, 3);
}

void StartScene::showInformation2() {
	removeChild(information);
	removeChild(menu);
	information = Sprite::create("information2.png");
	information->setPosition(winSize.width / 2, winSize.height / 2);
	addChild(information, 2);
	item = MenuItemLabel::create(Label::createWithSystemFont("OK", "Microsoft Yahei", 30.0f), CC_CALLBACK_0(StartScene::returnToMenu, this));
	menu = Menu::create(item, NULL);
	menu->setPosition(winSize.width / 2, winSize.height / 10);
	menu->alignItemsVerticallyWithPadding(item->getContentSize().height / 2);
	this->addChild(menu, 3);
}

void StartScene::returnToMenu() {
	auto scene = StartScene::createScene();
	Director::getInstance()->replaceScene(scene);
}

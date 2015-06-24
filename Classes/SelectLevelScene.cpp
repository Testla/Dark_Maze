#include "SelectLevelScene.h"
#pragma execution_character_set("utf-8")
USING_NS_CC;

Scene* SelectLevel::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = SelectLevel::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool SelectLevel::init() {
	if (!Layer::init())
	{
		return false;
	}

	dispatcher = Director::getInstance()->getEventDispatcher();
	winSize = Director::getInstance()->getWinSize();

	//Label::create()
	//MenuItemLabel::create()

	return true;
}

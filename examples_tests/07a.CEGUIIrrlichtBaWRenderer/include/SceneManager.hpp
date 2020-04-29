#ifndef _SCENE_MANAGER_HPP_INCLUDED_
#define _SCENE_MANAGER_HPP_INCLUDED_

#include <irrlicht.h>

class SceneManager
{
public:

	SceneManager(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device);
	virtual ~SceneManager();

private:

	irr::core::smart_refctd_ptr<irr::IrrlichtDevice> device;

};

#endif // _SCENE_MANAGER_HPP_INCLUDED_
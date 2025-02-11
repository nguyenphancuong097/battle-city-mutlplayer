﻿#include<PNet\Server.h>
#include "TankEnemy.h"



TankEnemy::TankEnemy(Sprite* sprite, Sound* sound, int _id, int level) : Tank(sprite, sound, _id)
{
	this->Tag = Object::enemy;
	this->Team = 3;//Màu xám
	this->Life = 1;
	this->Shoot = false;
	this->Level = level;
	this->runAi = true;
}


TankEnemy::~TankEnemy()
{
}

//HP
void  TankEnemy::CheckHP()
{
	this->HP--;
	if (this->HP > 0)
	{
		switch (this->HP)
		{
		case 1:
			this->Team = 3;
			break;

		case 2:
			this->Team = 2;
			break;

		case 3:
			this->Team = 1;
			break;

		default:
			this->Team = 3;
			break;
		}
		this->StateTank = Statetank::Standing;
	}
}

//New
void TankEnemy::New()
{
	Tank::New();
	//Tank
	this->TimeMove = 0.0f;
	this->TimeChangeMove = 0.0f;

	this->Team = 3;//Màu xám

	this->HP = 1;
	this->TimeShootColdown = 2* DEFAULT_SHOOT_COLDOWN;
	switch (Level)
	{
	case 4:
		this->TankSpeed = RunSpeed * 3 / 4;
		this->TimeShootColdown = 1.5f* DEFAULT_SHOOT_COLDOWN;
		break;

	case 5:
		this->TankSpeed = RunSpeed * 4 / 3;
		this->TimeShootColdown = 2.5f* DEFAULT_SHOOT_COLDOWN;
		break;

	case 7:
		/*this->HP = 4;
		this->CheckHP();*/
		this->TankSpeed = RunSpeed / 2;
		this->TimeShootColdown = DEFAULT_SHOOT_COLDOWN;
		break;

	default:
		break;
	}

	//Bullet
	Bullet *Bullet_0 = new Bullet(sprite, sound);
	this->ListBullet.push_back(Bullet_0);

	//New Item
	//if (rand() % 4 == 2)
	//{
	//	this->ItemTank->New();
	//}

	this->StateTank = Statetank::Appearing;
}

//Phím
void TankEnemy::AI()
{
	if (this->StateTank == Statetank::Appearing || this->StateTank == Statetank::Dying)
	{
		this->Shoot = false;
		return;
	}

	//Cho phép Tank bắn
	this->Shoot = true;

	//Kiểm tra chuyển hướng
	if (TimeMove >= TimeChangeMove)
	{
		//reset time move
		this->TimeMove = 0.0f;
		this->TimeChangeMove = 500 + rand() % (500 + 1);

		//ngẫu nhiên hướng di chuyển mới
		int randMove = (rand() % 999) % 4;
		switch (randMove)
		{
		case 0:
			this->StateTank = Statetank::RunningUp;
			break;

		case 1:
			this->StateTank = Statetank::RunningLeft;
			break;

		case 2:
			this->StateTank = Statetank::RunningDown;
			break;

		case 3:
			this->StateTank = Statetank::RunningRight;
			break;

		default:
			break;
		}
	}
}

//Đổi chuyển động
void TankEnemy::ChangeAnimation(float gameTime)
{
	if (this->StateTank == Statetank::Dying)
	{
		//Kiểm tra HP
		this->CheckHP();
	}
	this->TimeMove += gameTime;
	Tank::ChangeAnimation(gameTime);
}

void TankEnemy::OnCollision(Tank *tank_0, float gameTime)
{
	Object* object_0 = tank_0;
	Tank::OnCollision(object_0, gameTime);

	//Đồng đội bỏ qua
	if (tank_0->Tag != Object::enemy)
	for (size_t i = 0; i < ListBullet.size(); i++)
	{
		if (ListBullet.at(i)->GetState() == Bullet::Firing &&
			ListBullet.at(i)->OnCollision(tank_0, gameTime))
		{
			//Nếu không bất tử thì chết
			if (!tank_0->isImmortal())
			{
				tank_0->numBulletColision++;
				if (Server::serverPtr != NULL)
					tank_0->SetState(Statetank::Dying);
			}
			ListBullet.at(i)->SetState(Bullet::Bursting);
			if (Server::serverPtr != NULL)
			{
				ListBullet.at(i)->SendBurstingBullet(this->idNetWork);
			}
		}
	}
}

void TankEnemy::OnCollision(Object *object_0, float gameTime)
{
	Tank::OnCollision(object_0, gameTime);
}

//Update
void TankEnemy::Update(float gameTime)
{
	//Tank
	if (Server::serverPtr != NULL && runAi)
		this->AI();

	Tank::Update(gameTime);
}

//Render
void TankEnemy::Render(Viewport* viewport)
{
	Tank::Render(viewport);
}

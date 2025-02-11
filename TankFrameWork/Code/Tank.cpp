﻿#include<PNet\Server.h>
#include<PNet\Client.h>
#include<PNet\Packet.h>
#include<iostream>
#include "Tank.h"
#include"Map.h"
inline int RoundingNumber(float x, float number)
{
	int x1 = (x + number / 2) / number;
	return x1 * number;
}
Tank::Tank(Sprite* sprite, Sound* sound, int _id)
{
	this->sound = sound;
	this->sprite = sprite;
	this->TankAnimation = new Animation(this->sprite);
	this->StateTank = Statetank::Appearing;
	this->TimeBleed = 0.0f;
	this->TimeImmortal = 0;
	this->ReLoad = false;
	this->TankSpeed = RunSpeed;
	this->idNetWork = _id;
	this->numBulletColision = 0;
	this->TimeShootColdown = 0.75f* DEFAULT_SHOOT_COLDOWN;
	this->TimeShootLag = 0;
	this->AllowSetlag = false;
}

Tank::Tank(Tank * tank)
{
	Bullet *b = new Bullet(tank->ListBullet.at(0));
	this->ListBullet.push_back(b);
	CopyTank(tank);
	this->TankAnimation = NULL;
	this->sprite = tank->sprite;
	this->sound = tank->sound;

}

void Tank::CopyTank(Tank * tank)
{
	CopyObject(tank);
	this->AllowSetlag = tank->AllowSetlag;
	this->DirectionTank = tank->DirectionTank;
	this->idNetWork = tank->idNetWork;
	this->Immortal = tank->Immortal;
	this->Level = tank->Level;
	this->Life = tank->Life;
	this->numBulletColision = tank->numBulletColision;
	this->ReLoad = tank->ReLoad;
	this->Shoot = tank->Shoot;
	this->StateTank = tank->StateTank;
	this->TankSpeed = tank->TankSpeed;
	this->Team = tank->Team;
	this->TimeAppear = tank->TimeAppear;
	this->TimeBleed = tank->TimeBleed;
	this->TimeDie = tank->TimeDie;
	this->TimeImmortal = tank->TimeImmortal;
	this->TimeShoot = tank->TimeShoot;
	this->TimeShootColdown = tank->TimeShootColdown;
	this->TimeShootLag = tank->TimeShootLag;
	this->TimeMove = tank->TimeMove;
	this->TimeChangeMove = tank->TimeChangeMove;
	ListBullet.at(0)->CopyBullet(tank->ListBullet.at(0));

}

Tank::~Tank()
{
	if (TankAnimation != NULL)
		delete TankAnimation;
}

vector <Bullet*> Tank::GetListBullet()
{
	return this->ListBullet;
}

int Tank::GetTeam()
{
	return this->Team;
}

int Tank::GetIDNetWork()
{
	return this->idNetWork;
}

int Tank::GetLevel()
{
	return this->Level;
}

int Tank::GetDirection()
{
	return this->DirectionTank;
}

D3DXVECTOR2 Tank::GetVelocity(Statetank state)
{
	switch (state)
	{
	case PNet::Appearing:
	case PNet::Bleeing:
	case PNet::Standing:
	case PNet::Dying:
	case PNet::Dead:
		return D3DXVECTOR2(0, 0);
		break;
	case PNet::RunningUp:
		return D3DXVECTOR2(0, TankSpeed);
		break;
	case PNet::RunningLeft:
		return D3DXVECTOR2(-TankSpeed, 0);
		break;
	case PNet::RunningDown:
		return D3DXVECTOR2(0, -TankSpeed);
		break;
	case PNet::RunningRight:
		return D3DXVECTOR2(TankSpeed, 0);
		break;
	default:
		return D3DXVECTOR2(0, 0);
		break;
	}
	return D3DXVECTOR2();
}

void Tank::SetLevel(int level)
{
	this->Level = level;
}

void Tank::SetLife(int life)
{
	this->Life = life;
}

void Tank::SetTimeShootLag(float lag)
{
	if (AllowSetlag)
	{
		AllowSetlag = false;
		TimeShootLag = lag;
	}
}

int Tank::GetLife()
{
	return this->Life;
}

bool Tank::isImmortal()
{
	return Immortal;
}
void Tank::SetTimeImmortal(float time)
{
	this->TimeImmortal = time;
}
void Tank::SetImmortal(bool immortal)
{
	this->Immortal = immortal;
}

void Tank::SetShoot(bool isShoot, BulletProperties bulletP)
{
	this->Shoot = isShoot;
	queue_bulletP.clear();
	queue_bulletP.push_back(bulletP);
}

void Tank::SetBurstingBullet(long timeSend)
{
	if (ListBullet.size() <= 0)
		return;
	Bullet *b = ListBullet.at(0);
	if (b->GetState() == Bullet::Bursting)
		return;
	long current = GetTickCount();
	float lag = ((float)(current + Client::timeDifference - timeSend));
	b->SetState(Bullet::Bursting);
	b->SetTimeBurst(lag);
}

//Lấy trạng thái
Statetank Tank::GetState()
{
	return this->StateTank;
}
//Set trạng thái
void Tank::SetState(Statetank state)
{
	this->StateTank = state;
}
//New
void Tank::New()
{
	//xóa đạn cũ
	for (size_t i = 0; i < ListBullet.size(); i++)
	{
		Bullet* b_0 = ListBullet.at(i);
		delete b_0;
	}
	this->ListBullet.clear();
	//new mới thông số
	this->position = this->positionStart;
	this->Immortal = false;
	this->AllowDraw = true;
	this->TimeAppear = 0.0f;
	this->TimeDie = 0.0f;
	this->TimeShoot = 0.0f;
	this->numBulletColision = 0;
}
//Đổi chuyển động
void Tank::ChangeAnimation(float gameTime)
{
	this->SetBound(15, 15);
	//có 8 sprite di chuyển và 64 sprite cho 1 màu (player)
	int tank_sprite = (8 * this->Level) + (64 * this->Team);

	//Chuyển hình ảnh
	switch (StateTank)
	{
		//Xuất hiện
	case Statetank::Appearing:
	{
		this->SetBoundZero();
		this->SetVelocity(0.0f, 0.0f);
		if (TankAnimation != NULL)
			this->TankAnimation->SetFrame(this->position, false, 100, 261, 264);
		//Animation lặp lại 3 lần vào trạng thái đứng yên
		this->TimeAppear += gameTime;
		if (TimeAppear >= 1500 && Server::serverPtr != NULL)
		{
			//Sprite sau khi xuất hiện vào game
			this->StateTank = Statetank::Standing;
			//team 0 thì hướng lên
			if (this->Team == 0)
			{
				this->DirectionTank = 0;
			}
			else
			{
				this->DirectionTank = 2;
			}

			if (this->Tag == Object::player)
				this->Immortal = true;
		}
		break;
	}
	//Kiểm tra di chuyển
	case Statetank::Standing:
	{
		this->SetVelocity(0.0f, 0.0f);
		//Đứng yên theo hướng vừa Run
		if (TankAnimation != NULL)
			this->TankAnimation->SetFrame(this->position, false, 0, DirectionTank * 2 + tank_sprite, DirectionTank * 2 + tank_sprite);
		break;
	}
	case Statetank::RunningUp:
	{
		float pos_round = 100 * (RoundingNumber(position.x, 8) - this->position.x);
		if (pos_round != 0)
			this->SetVelocity(pos_round / gameTime, 0.0f);
		else
			this->SetVelocity(0.0f, TankSpeed);

		this->DirectionTank = 0;
		//Di chuyển lên frame 0, 1 delay 100
		if (TankAnimation)
			this->TankAnimation->SetFrame(this->position, false, RunDelay, 0 + tank_sprite, 1 + tank_sprite);
		break;
	}
	case Statetank::RunningLeft:
	{
		float pos_round = 100 * (RoundingNumber(position.y, 8) - this->position.y);
		if (pos_round != 0)
			this->SetVelocity(0.0f, pos_round / gameTime);
		else
			this->SetVelocity(-TankSpeed, 0.0f);

		this->DirectionTank = 1;
		//Di chuyển trái frame 2, 3 delay 100
		if (TankAnimation != NULL)
			this->TankAnimation->SetFrame(this->position, false, RunDelay, 2 + tank_sprite, 3 + tank_sprite);
		break;
	}
	case Statetank::RunningDown:
	{
		float pos_round = 100 * (RoundingNumber(position.x, 8) - this->position.x);
		if (pos_round != 0)
			this->SetVelocity(pos_round / gameTime, 0.0f);
		else
			this->SetVelocity(0.0f, -TankSpeed);

		this->DirectionTank = 2;
		//Di chuyển xuống frame 4, 5 delay 100
		if (TankAnimation != NULL)
			this->TankAnimation->SetFrame(this->position, false, RunDelay, 4 + tank_sprite, 5 + tank_sprite);
		break;
	}
	case Statetank::RunningRight:
	{
		float pos_round = 100 * (RoundingNumber(position.y, 8) - this->position.y);
		if (pos_round != 0)
			this->SetVelocity(0.0f, pos_round / gameTime);
		else
			this->SetVelocity(TankSpeed, 0.0f);

		this->DirectionTank = 3;
		//Di chuyển phải frame 6, 7 delay 100
		if (TankAnimation != NULL)
			this->TankAnimation->SetFrame(this->position, false, RunDelay, 6 + tank_sprite, 7 + tank_sprite);
		break;
	}
	//Bị đồng đội bắn trúng
	case Statetank::Bleeing:
	{
		this->SetVelocity(0.0f, 0.0f);
		this->TimeBleed += gameTime;
		int TimePause = TimeBleed / 300;
		if (TimeBleed >= 3000.0f && Server::serverPtr != NULL)
		{
			this->AllowDraw = true;
			this->TimeBleed = 0.0f;
			this->StateTank = Statetank::Standing;
			break;
		}

		if (TimePause % 2 == 0)
		{
			this->AllowDraw = true;
		}
		else if (TimePause % 2 == 1)
		{
			this->AllowDraw = false;
		}
		break;
	}
	//Đang chết
	case Statetank::Dying:
	{
		if (this->numBulletColision > 0)
			this->SetBoundZero();
		this->SetVelocity(0.0f, 0.0f);
		if (TankAnimation != NULL)
			this->TankAnimation->SetFrame(this->position, false, DieDelay, 306, 311);
		//
		this->TimeDie += gameTime;
		if (TimeDie >= 1000)
		{
			if (Server::serverPtr != NULL)
				this->New();
		}
		break;
	}
	//Chết
	case Statetank::Dead:
	{
		this->SetBoundZero();
		this->SetVelocity(0.0f, 0.0f);
		this->AllowDraw = false;
		break;
	}

	default:
		break;
	}

	//Nếu đang bắn
	if (Shoot && !ReLoad)
	{
		for (size_t i = 0; i < ListBullet.size(); i++)
		{
			if (!ListBullet.at(i)->GetAllowDraw())
			{
				if (Server::serverPtr != NULL)
					ServerShoot(gameTime, i);
				else
					ClientShoot(gameTime, i);
			}
		}
	}
	//Nạp đạn
	if (ReLoad)
	{
		this->TimeShoot += gameTime;
		if (TimeShoot >= 0.0f)
		{
			ReLoad = false;
			if (this->Tag == Object::player)
				this->AllowSetlag = true;
			this->TimeShoot = TimeShootColdown;
		}

	}
}

void Tank::ServerShoot(float gametime, int bulletID)
{
	this->Shoot = false;
	this->ReLoad = true;

	ListBullet.at(bulletID)->NewBullet(this->position, this->DirectionTank, this->Level);
	this->map->OnCollision(ListBullet.at(bulletID), this->Level, TimeShootLag);
	ListBullet.at(bulletID)->Update(TimeShootLag);
	this->TimeShoot += TimeShootLag;
	// Server send 
	{
		Packet p(PacketType::PT_Bullet_Shoot);
		long timeSend = GetTickCount();
		uint8_t tankID = (uint8_t)GetIDNetWork();
		int x, y;
		x = ListBullet.at(bulletID)->GetPosition().x * 10;
		y = ListBullet.at(bulletID)->GetPosition().y * 10;
		p.write_bits(true, 1);
		p.write_bits(tankID, 3);
		p.write_bits(timeSend, sizeof(long) * 8);
		p.write_bits(x, 12);
		p.write_bits(y, 12);
		p.write_bits(this->DirectionTank, 2);
		ListBullet.at(bulletID)->gamePacket.Append(p);
	}
}

void Tank::ClientShoot(float gametime, int bulletID)
{
	if (queue_bulletP.size() <= 0)
		return;
	this->Shoot = false;
	this->ReLoad = true;
	BulletProperties bulletP = queue_bulletP.front();
	ListBullet.at(bulletID)->NewBullet(D3DXVECTOR2(bulletP.x, bulletP.y),
		bulletP.direct, this->Level);
	long current = GetTickCount();
	float lag = ((float)(current + Client::timeDifference - bulletP.timeSend));
	this->map->OnCollision(ListBullet.at(bulletID), this->Level, lag);
	ListBullet.at(bulletID)->Update(lag);
	this->TimeShoot += lag;

	queue_bulletP.clear();
}

TankProperties Tank::GetProperties()
{
	TankProperties p;
	p.x = this->position.x;
	p.y = this->position.y;
	p.state = this->GetState();
	p.direct = this->DirectionTank;
	p.dx = (int)(velocity.x / TankSpeed);
	p.dy = (int)(velocity.y / TankSpeed);
	return p;
}

void Tank::SetProperties(TankProperties p, float lag)
{
	if (p.state == Statetank::Standing)
	{
		if (this->StateTank == Statetank::Appearing)
		{
			this->StateTank = Statetank::Standing;
			//team 0 thì hướng lên
			if (this->Team == 0)
			{
				this->DirectionTank = 0;
			}
			else
			{
				this->DirectionTank = 2;
			}

			if (this->Tag == Object::player)
			{
				this->Immortal = true;
				this->TimeImmortal += lag;
			}
		}
		if (this->StateTank == Statetank::Bleeing)
		{
			this->AllowDraw = true;
			this->TimeBleed += lag;
			this->StateTank = Statetank::Standing;
		}
	}

	if (p.state == Statetank::Dying && this->StateTank != Statetank::Dying)
	{
		TimeDie += lag;
	}
	if (p.state == Statetank::Appearing && this->StateTank == Statetank::Dying)
	{
		this->New();
		this->TimeAppear += lag;
	}

	float d = VectorDistance(this->position,
		D3DXVECTOR2(p.x + lag / 100 * p.dx, p.y + lag / 100 * p.dx));
	D3DXVECTOR2 pos = GetPosition();
	if (d > MaxDistance)
	{
		this->SetPosition(D3DXVECTOR2(p.x, p.y));
		SetVelocity(D3DXVECTOR2(TankSpeed* p.dx, TankSpeed* p.dy));
		map->OnCollision(this, lag);
		Object::Update(lag);
	}

	this->StateTank = p.state;
	this->DirectionTank = p.direct;
}

void Tank::SetBulletProperties(TankBulletProperties p, float lag)
{
	if (ListBullet.size() <= 0)
		return;

	if (ListBullet.at(0)->GetState() != p.state && ListBullet.at(0)->GetState() == Bullet::Bursting)
	{
		if (!ListBullet.at(0)->GetAllowDraw())
		{
			ListBullet.at(0)->NewBullet(D3DXVECTOR2(p.x, p.y),
				p.direct, this->Level);
			D3DXVECTOR2 velocity = ListBullet.at(0)->GetVelocity();
			ListBullet.at(0)->SetPositionX(p.x + lag * velocity.x);
			ListBullet.at(0)->SetPositionY(p.y + lag * velocity.y);
		}
		else
		{
			if (VectorDistance(ListBullet.at(0)->GetPosition(),
				D3DXVECTOR2(p.x + lag * velocity.x, p.y + lag * velocity.y)) > MaxDistance)
			{
				D3DXVECTOR2 velocity = ListBullet.at(0)->GetVelocity();
				ListBullet.at(0)->SetPositionX(p.x + lag * velocity.x);
				ListBullet.at(0)->SetPositionY(p.y + lag * velocity.y);
			}
		}
	}
}

//Va chạm
void Tank::OnCollision(Tank *tank_0, float gameTime)
{
}

void Tank::OnCollision(Object *object_0, float gameTime)
{
	D3DXVECTOR2 distance = this->Distance(gameTime) - object_0->Distance(gameTime);
	RECT board = Collision::GetBoard(this->GetBound(), distance);
	//Nếu tank trong board
	if (Collision::isCollision(board, object_0->GetBound()))
	{
		//Lấy cạnh va chạm
		D3DXVECTOR2 SideCollision;
		//lấy thời gian va chạm
		float Time = Collision::CollisionAABB(this->GetBound(), object_0->GetBound(), distance, SideCollision);

		//bé hơn 1 thì có va chạm
		if (Time < 1.0f)
		{

			if (SideCollision.x != 0.0f)
			{
				this->position.x += this->Distance(gameTime).x*Time;
				this->velocity.x = 0.0f;
			}

			if (SideCollision.y != 0.0f)
			{
				this->position.y += this->Distance(gameTime).y*Time;
				this->velocity.y = 0.0f;
			}
		}
	}
}
//Update
void Tank::Update(float gameTime)
{
	//Tank
	Object::Update(gameTime);
	if (TankAnimation)
		this->TankAnimation->Update(gameTime);

	//Bullet
	for (size_t i = 0; i < ListBullet.size(); i++)
	{
		ListBullet.at(i)->Update(gameTime);
	}
}

//Render
void Tank::Render(Viewport* viewport)
{
	//Vẽ đạn
	for (size_t i = 0; i < ListBullet.size(); i++)
	{
		ListBullet.at(i)->Render(viewport);
	}
	//Không cho vẽ
	if (AllowDraw)
		this->TankAnimation->Render(viewport);
}
#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Plotter.h"
#include "/home/codeleaded/System/Static/Library/Random.h"


#define BOID_SIZE 		0.004f
#define BOID_VISION 	0.1f
#define BOID_FOV 		(F32_PI * 1.5f)
#define BOID_VELO		0.2f

typedef struct Boid{
	Vec2 p;
	Vec2 v;
	Vec2 a;
} Boid;

void Boid_Reset(Boid* b){
	b->a = (Vec2){ 0.0f,0.0f };
}
void Boid_Move(Boid* b,float t){
	b->v = Vec2_Add(b->v,Vec2_Mulf(b->a,t));
	b->v = Vec2_Mulf(Vec2_Norm(b->v),BOID_VELO);
	b->p = Vec2_Add(b->p,Vec2_Mulf(b->v,t));

	if(b->p.x < BOID_SIZE) 			b->p.x = BOID_SIZE;
	if(b->p.y < BOID_SIZE) 			b->p.y = BOID_SIZE;
	if(b->p.x > 1.0f - BOID_SIZE) 	b->p.x = 1.0f - BOID_SIZE;
	if(b->p.x > 1.0f - BOID_SIZE) 	b->p.x = 1.0f - BOID_SIZE;
}
void Boid_Interact(Boid* b,Boid* b2,float t){
	const Vec2 dir = Vec2_Sub(b->p,b2->p);
	const float d = Vec2_Mag(dir);
	const float qu = Vec2_Dot(dir,b->v) / (d * Vec2_Mag(b->v));

	if(d < BOID_VISION && qu > cosf(BOID_FOV)){
		b->a = Vec2_Add(b->a,Vec2_Divf(dir,d * d));
	}
}
Vec2 Boid_Obstacle(Boid* b,Vec2 p){
	const Vec2 dir = Vec2_Sub(p,b->p); 
	const Vec2 f = Vec2_Divf(dir,-Vec2_Mag2(dir));
	return f;
}
Vec2 Boid_Target(Boid* b,Vec2 p){
	const Vec2 dir = Vec2_Sub(p,b->p); 
	return Vec2_Mulf(dir,10.0f);
}
void Boid_Collision(Boid* b){
	b->a = Vec2_Add(Vec2_Mulf(b->a,0.1f),Boid_Obstacle(b,(Vec2){ 0.0f,b->p.y }));
	b->a = Vec2_Add(Vec2_Mulf(b->a,0.1f),Boid_Obstacle(b,(Vec2){ 1.0f,b->p.y }));
	b->a = Vec2_Add(Vec2_Mulf(b->a,0.1f),Boid_Obstacle(b,(Vec2){ b->p.x,0.0f }));
	b->a = Vec2_Add(Vec2_Mulf(b->a,0.1f),Boid_Obstacle(b,(Vec2){ b->p.x,1.0f }));
}
void Boid_Render(unsigned int* Target,int Width,int Height,Boid* b){
	const Vec2 rot = Vec2_Mulf(Vec2_Norm(b->v),BOID_SIZE);
	const Vec2 perp = Vec2_Perp(rot);
	const Vec2 front = Vec2_Mulf(rot,2.0f);

	const Vec2 p1 = Vec2_Add(b->p,front);
	const Vec2 p2 = Vec2_Add(Vec2_Sub(b->p,front),perp);
	const Vec2 p3 = Vec2_Sub(Vec2_Sub(b->p,front),perp);
	
	const Vec2 sp1 = Vec2_Mul(p1,(Vec2){ Width,Height });
	const Vec2 sp2 = Vec2_Mul(p2,(Vec2){ Width,Height });
	const Vec2 sp3 = Vec2_Mul(p3,(Vec2){ Width,Height });
	Triangle_RenderX(Target,Width,Height,sp1,sp2,sp3,WHITE);

	//Circle_RenderXWire(Target,Width,Height,Vec2_Mul(b->p,(Vec2){ Width,Height }),BOID_VISION * (float)Width,WHITE,1.0f);
}



Vector boids;

void Setup(AlxWindow* w){
	boids = Vector_New(sizeof(Boid));
	
	for(int i = 0;i<500;i++){
		Vector_Push(&boids,(Boid[]){{
			.p = (Vec2){Random_f64_MinMax(0.0f,1.0f),Random_f64_MinMax(0.0f,1.0f)},
			.v = (Vec2){0.0f,0.0f},
			.a = (Vec2){0.0f,0.0f}
		}});
	}
}
void Update(AlxWindow* w){
	Vec2 pavg = {0.0f,0.0f};
	Vec2 vavg = {0.0f,0.0f};
	
	for(int i = 0;i<boids.size;i++){
		Boid* b = (Boid*)Vector_Get(&boids,i);
		pavg = Vec2_Add(pavg,b->p);
		vavg = Vec2_Add(vavg,b->v);
	}
	pavg = Vec2_Divf(pavg,boids.size);
	vavg = Vec2_Divf(vavg,boids.size);


	for(int i = 0;i<boids.size;i++){
		Boid* b = (Boid*)Vector_Get(&boids,i);
		
		Boid_Reset(b);

		for(int j = 0;j<boids.size;j++){
			if(i==j) continue;

			Boid* b2 = (Boid*)Vector_Get(&boids,j);
			Boid_Interact(b,b2,w->ElapsedTime);
		}

		b->a = Vec2_Add(Vec2_Mulf(b->a,1.5f),Vec2_Add(pavg,vavg));
		Boid_Collision(b);

		if(Stroke(ALX_MOUSE_L).DOWN)
			b->a = Vec2_Add(b->a,Boid_Target(b,Vec2_Div(GetMouse(),(Vec2){ GetWidth(),GetHeight() })));

		Boid_Move(b,w->ElapsedTime);
	}
	
	Clear(DARK_BLUE);

	for(int i = 0;i<boids.size;i++){
		Boid* b = (Boid*)Vector_Get(&boids,i);
		Boid_Render(WINDOW_STD_ARGS,b);
	}
}
void Delete(AlxWindow* w){
	Vector_Free(&boids);
}

int main(){
    if(Create("Boids",1400,1400,1,1,Setup,Update,Delete))
        Start();
    return 0;
}
#include "Framework.h"
#include "Fixity.h"

Fixity::Fixity()
{

}

Fixity::~Fixity()
{
}

void Fixity::Update()
{
	Rotation();
	View();
}

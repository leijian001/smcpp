#include <stdio.h>

#include "../smcpp.h"

enum
{
	CHAR_SIG = SM::USER_SIG,
};

class E: public SM::Event
{
public:
	E(SM::Singal sig): Event(sig){}
	char c;
};


class A: public SM::Hsm
{
public:
	A(): Hsm(init)
	{
	}

private:
	int a;

	static int init(SM::SMAttr *const hsm, SM::Event *const e)
	{
		printf("init\n");
		return hsm->tran(top);
	}

	static int top(SM::SMAttr *const hsm, SM::Event *const e)
	{
		A *const p = static_cast<A *>(SM::Hsm::hsm_entry(hsm));

		switch(e->sig)
		{
		case SM::ENTRY_SIG:
			printf("top entry\n");
			return hsm->handled();
		case SM::EXIT_SIG:
			printf("top exit\n");
			return hsm->handled();
		case SM::INIT_SIG:
			printf("top init\n");
			return hsm->tran(s0);
		}

		return hsm->supper(hsm_top);
	}

	static int s0(SM::SMAttr *const hsm, SM::Event *const event)
	{
		E *e = static_cast<E *>(event);

		switch(e->sig)
		{
		case SM::ENTRY_SIG:
			printf("s0 entry\n");
			return hsm->handled();
		case SM::EXIT_SIG:
			printf("s0 exit\n");
			return hsm->handled();
		case SM::INIT_SIG:
			printf("s0 init\n");
			return hsm->tran(s2);

		case CHAR_SIG:
			if ('1' == e->c)
			{
				return hsm->tran(s1);
			}
			return hsm->handled();
		}

		return hsm->supper(top);
	}

	static int s1(SM::SMAttr *const hsm, SM::Event *const event)
	{

		switch(event->sig)
		{
		case SM::ENTRY_SIG:
			printf("s1 entry\n");
			return hsm->handled();
		case SM::EXIT_SIG:
			printf("s1 exit\n");
			return hsm->handled();
		case SM::INIT_SIG:
			printf("s1 init\n");
			return hsm->tran(s3);

		case CHAR_SIG:
		{
			E *e = static_cast<E *>(event);
			if ('1' == e->c)
			{
				return hsm->tran(s0);
			}
			return hsm->handled();
		}
		}

		return hsm->supper(top);
	}

	static int s2(SM::SMAttr *const hsm, SM::Event *const event)
	{
		switch(event->sig)
		{
		case SM::ENTRY_SIG:
			printf("s2 entry\n");
			return hsm->handled();
		case SM::EXIT_SIG:
			printf("s2 exit\n");
			return hsm->handled();
		}

		return hsm->supper(s0);
	}

	static int s3(SM::SMAttr *const hsm, SM::Event *const event)
	{
		switch(event->sig)
		{
		case SM::ENTRY_SIG:
			printf("s3 entry\n");
			return hsm->handled();
		case SM::EXIT_SIG:
			printf("s3 exit\n");
			return hsm->handled();
		}

		return hsm->supper(s1);
	}
};


int main(int argc, char *argv[])
{
	A a;

	a.initial();

	E e(CHAR_SIG);
	for(;;)
	{
		scanf("%c", &e.c);
		if (e.c == 'q')
		{
			break;
		}

		a.dispatch(&e);
	}

	return 0;
}

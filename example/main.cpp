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

	static int init(SM::Attr &hsm, SM::Event &e)
	{
		printf("init\n");
		return hsm.tran(top);
	}

	static int top(SM::Attr &hsm, SM::Event &e)
	{
		auto &p = dynamic_cast<A&>(hsm);
		(void)p;

		switch(e.sig)
		{
		case SM::ENTRY_SIG:
			printf("top entry\n");
			return hsm.handled();
		case SM::EXIT_SIG:
			printf("top exit\n");
			return hsm.handled();
		case SM::INIT_SIG:
			printf("top init\n");
			return hsm.tran(s0);
		}

		return hsm.supper(hsm_top);
	}

	static int s0(SM::Attr &hsm, SM::Event &event)
	{

		switch(event.sig)
		{
		case SM::ENTRY_SIG:
			printf("s0 entry\n");
			return hsm.handled();
		case SM::EXIT_SIG:
			printf("s0 exit\n");
			return hsm.handled();
		case SM::INIT_SIG:
			printf("s0 init\n");
			return hsm.tran(s2);

		case CHAR_SIG:
		{
			auto e = dynamic_cast<E&>(event);
			if ('1' == e.c)
			{
				return hsm.tran(s1);
			}
			return hsm.handled();
		}
		}

		return hsm.supper(top);
	}

	static int s1(SM::Attr &hsm, SM::Event &event)
	{

		switch(event.sig)
		{
		case SM::ENTRY_SIG:
			printf("s1 entry\n");
			return hsm.handled();
		case SM::EXIT_SIG:
			printf("s1 exit\n");
			return hsm.handled();
		case SM::INIT_SIG:
			printf("s1 init\n");
			return hsm.tran(s3);

		case CHAR_SIG:
		{
			auto e = dynamic_cast<E&>(event);
			if ('1' == e.c)
			{
				return hsm.tran(s0);
			}
			return hsm.handled();
		}
		}

		return hsm.supper(top);
	}

	static int s2(SM::Attr &hsm, SM::Event &event)
	{
		switch(event.sig)
		{
		case SM::ENTRY_SIG:
			printf("s2 entry\n");
			return hsm.handled();
		case SM::EXIT_SIG:
			printf("s2 exit\n");
			return hsm.handled();
		}

		return hsm.supper(s0);
	}

	static int s3(SM::Attr &hsm, SM::Event &event)
	{
		switch(event.sig)
		{
		case SM::ENTRY_SIG:
			printf("s3 entry\n");
			return hsm.handled();
		case SM::EXIT_SIG:
			printf("s3 exit\n");
			return hsm.handled();
		}

		return hsm.supper(s1);
	}
};


int main(int argc, char *argv[])
{
	A a;

	a.start();

	E e(CHAR_SIG);
	for(;;)
	{
		scanf("%c", &e.c);
		if (e.c == 'q')
		{
			break;
		}

		a.dispatch(e);
	}

	return 0;
}

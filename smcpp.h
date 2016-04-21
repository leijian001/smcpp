#ifndef __SMCPP_H__
#define __SMCPP_H__

#include <stddef.h>

namespace SM
{

#define CONFIG_SM_DEBUG 		1

#ifndef CONFIG_SM_FSM
#define CONFIG_SM_FSM 			1
#endif

#ifndef CONFIG_SM_HSM
#define CONFIG_SM_HSM 			1
#endif

#if ! (CONFIG_SM_FSM || CONFIG_SM_HSM)
#error "FSM and HSM must chose one!"
#endif

#ifndef SM_MAX_NEST_DEPTH
#define SM_MAX_NEST_DEPTH 		6
#endif

#if CONFIG_SM_DEBUG
#define SM_ASSERT(cond) 	do{ if(!(cond)){ while(1); } }while(0)
#else
#define SM_ASSERT(cond) 	/* NULL */
#endif

typedef int Singal;
enum RESERVED_SIG
{
	EMPTY_SIG = -4,
	ENTRY_SIG = -3,
	EXIT_SIG  = -2,
	INIT_SIG  = -1,
	USER_SIG  =  0,
};

/**
 * @brief 事件类
 */
class Event
{
public:
	Event(Singal const s): sig(s){}

	Singal sig;
};
const Event RESERVED_EVENT[] =
{
	Event(EMPTY_SIG),
	Event(ENTRY_SIG),
	Event(EXIT_SIG),
	Event(INIT_SIG),
	Event(USER_SIG),
};


/**
 * @bref 状态处理函数的返回值类型
 *
 */
enum
{
	RET_HANDLED,
	RET_IGNORE,
	RET_UNHANDLED,

	RET_TRAN,
	RET_SUPER,
};

class SMAttr;
typedef int (*StateHander)(SMAttr *const sm, Event *const e);

class SMAttr
{
public:
	StateHander m_state;
	StateHander m_temp;

	inline int trig(StateHander state, Singal sig)
	{
		return state(this, const_cast<Event *const>(&RESERVED_EVENT[4+sig]));
	}
	inline int entry(StateHander state)
	{
		return trig(state, ENTRY_SIG);
	}
	inline int exit(StateHander state)
	{
		return trig(state, EXIT_SIG);
	}


	inline int handled(void)
	{
		return RET_HANDLED;
	}
	inline int ignore(void)
	{
		return RET_IGNORE;
	}
	inline int unhandled(void)
	{
		return RET_UNHANDLED;
	}
	inline int tran(StateHander target)
	{
		m_temp = target;
		return RET_TRAN;
	}
	inline int supper(StateHander target)
	{
		m_temp = target;
		return RET_SUPER;
	}
};

#if CONFIG_SM_FSM
class Fsm: private SMAttr
{
public:
	Fsm(StateHander init)
	{
		SM_ASSERT(init);

		m_state = 0;
		m_temp = init;
	}

	int initial(Event *const e = 0)
	{
		int ret;

		SM_ASSERT(m_temp);

		ret = m_temp(this, e);
		if (RET_TRAN != ret)
		{
			return ret;
		}

		entry(m_temp);

		m_state = m_temp;

		return ret;
	}

	void dispatch(Event *const e)
	{
		int ret;

		SM_ASSERT(m_state == m_temp);

		ret = m_temp(this, e);
		if (ret == RET_TRAN)
		{
			exit(m_state);
			entry(m_temp);
			m_state = m_temp;
		}
	}
};
#endif

#if CONFIG_SM_HSM
class Hsm: private SMAttr
{
public:
	Hsm(StateHander init)
	{
		SM_ASSERT(init);

		m_state = hsm_top;
		m_temp  = init;
	}

	void initial(Event *const e = 0)
	{
		int ret;
		int ip;

		StateHander path[SM_MAX_NEST_DEPTH];
		StateHander t = m_state;

		SM_ASSERT(0 != m_temp);
		SM_ASSERT(hsm_top == t);

		ret = (m_temp)(this, e);
		SM_ASSERT(RET_TRAN == ret);

		do
		{
			ip = 0;

			path[0] = m_temp;
			trig(m_temp, EMPTY_SIG);
			while( t != m_temp )
			{
				path[++ip] = m_temp;
				trig(m_temp, EMPTY_SIG);
			}
			m_temp = path[0];

			SM_ASSERT(ip < SM_MAX_NEST_DEPTH);

			do
			{
				entry(path[ip--]);
			}while(ip >= 0);

			t = path[0];
		}while(RET_TRAN == trig(t, INIT_SIG));

		m_temp = t;
		m_state = m_temp;
	}

	void dispatch(Event *const e)
	{
		StateHander t = m_state;
		StateHander s;

		int ret;

		// 状态必须稳定
		SM_ASSERT(m_state == m_temp);

		/* process the event hierarchically... */
		// 事件递归触发, 直到某个状态处理该事件
		do
		{
			s = m_temp;
			ret = s(this, e); 	// 调用状态处理函数
			if(RET_UNHANDLED == ret)
			{
				ret = trig(s, EMPTY_SIG);
			}
		}while(RET_SUPER == ret);

		// 如果发生状态转换
		if(RET_TRAN == ret)
		{
			StateHander path[SM_MAX_NEST_DEPTH];
			signed char ip = -1;

			path[0] = m_temp; 	// 状态转换的目的状态
			path[1] = t; 			// 状态转换的源状态

			/* exit current state to transition source s... */
			for( ; s != t; t = m_temp)
			{
				ret = exit(t);
				if(RET_HANDLED == ret)
				{
					trig(t, EMPTY_SIG);
				}
			}

			ip = find_path(path[0], s, path);

			for(; ip>=0; ip--)
			{
				entry(path[ip]);
			}

			t = path[0];
			m_temp = t;

			/* drill into the target hierarchy... */
			while( RET_TRAN == trig(t, INIT_SIG) )
			{
				ip = 0;
				path[0] = m_temp;

				trig(m_temp, EMPTY_SIG);
				while(t != m_temp)
				{
					path[++ip] = m_temp;
					trig(m_temp, EMPTY_SIG);
				}
				m_temp = path[0];

				SM_ASSERT(ip < SM_MAX_NEST_DEPTH);

				do
				{
					entry(path[ip--]);
				}while(ip >= 0);

				t = path[0];
			}// end: while( SM_RET_TRAN == SM_TRIG(me, t, SM_INIT_SIG) )
		} // end: if(SM_RET_TRAN == ret)

		m_temp = t;
		m_state = t;
	}

	//! 层次状态机根状态
	static int hsm_top(SMAttr *const sm, Event *const e)
	{
		(void)sm;
		(void)e;
		return RET_IGNORE;
	}

private:

	int find_path(StateHander t, StateHander s, StateHander path[SM_MAX_NEST_DEPTH])
	{
		int ip = -1;
		int iq;
		int ret;

		/* (a) check source==target (transition to self) */
		if( s == t)
		{
			exit(s);
			ip = 0;

			goto hsm_find_path_end;
		}

		trig(t, EMPTY_SIG);
		t = m_temp;

		/* (b) check source==target->super */
		if( s == t )
		{
			ip = 0;
			goto hsm_find_path_end;
		}

		trig(s, EMPTY_SIG);

		/* (c) check source->super==target->super */
		if(m_temp == t)
		{
			exit(s);
			ip = 0;
			goto hsm_find_path_end;
		}

		/* (d) check source->super==target */
		if( m_temp == path[0] )
		{
			exit(s);
			goto hsm_find_path_end;
		}

		/* (e) check rest of source==target->super->super..
		 * and store the entry path along the way
		 */
		ip = 1;
		iq = 0;
		path[1] = t;
		t = m_temp;

		/* find target->super->super... */
		ret = trig(path[1], EMPTY_SIG);
		while(RET_SUPER == ret)
		{
			path[++ip] = m_temp;
			if(s == m_temp)
			{
				iq = 1;
				SM_ASSERT(ip < SM_MAX_NEST_DEPTH);
				ip--;

				ret = RET_HANDLED;
			}
			else
			{
				ret = trig(m_temp, EMPTY_SIG);
			}
		}

		/* the LCA not found yet? */
		if(0 == iq)
		{
			SM_ASSERT(ip < SM_MAX_NEST_DEPTH);

			exit(s);

			/* (f) check the rest of source->super
			 *                  == target->super->super...
			 */
			iq = ip;
			ret = RET_IGNORE; /* LCA NOT found */
			do
			{
				s = path[iq];
				/* is this the LCA? */
				if(t == s)
				{
					ret = RET_HANDLED;

					ip = iq - 1;
					iq = -1;
				}
				else
				{
					iq--; /* try lower superstate of target */
				}
			}while(iq >= 0);

			/* LCA not found? */
			if( RET_HANDLED != ret )
			{
				/* (g) check each source->super->...
				 * for each target->super...
				 */
				ret = RET_IGNORE;
				do
				{
					if(RET_HANDLED  == exit(t))
					{
						trig(t, EMPTY_SIG);
					}
					t = m_temp;
					iq = ip;
					do
					{
						s = path[iq];
						if( t == s)
						{
							ip = iq -1;
							iq = -1;

							ret = RET_HANDLED; /* break */
						}
						else
						{
							iq--;
						}
					}while(iq >= 0);
				}while(RET_HANDLED != ret);
			}
		}

hsm_find_path_end:
		return ip;
	}
};
#endif	// CONFIG_SM_HSM

} // end namespace

#endif

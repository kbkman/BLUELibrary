#ifndef __BLUERANDOM_H__
#define __BLUERANDOM_H__

#include <BLUEDef.h>
#include <time.h>

#define _BLUERANDOM_A              48271
#define _BLUERANDOM_M              2147483647
#define _BLUERANDOM_Q              44488                //相当于(_BLUERANDOM_M / _BLUERANDOM_A)
#define _BLUERANDOM_R               3399                 //相当于(_BLUERANDOM_M % _BLUERANDOM_A)

/**
类名：随机数生成器
功能：无
说明：无
版本：1.0.0.1
作者：BLUE
*/
class CBLUERandom
{
public :
	explicit CBLUERandom::CBLUERandom(int initialVal) {Init(initialVal);}
	CBLUERandom::CBLUERandom(void){Init((int)::time(BLUENULL));}

	int CBLUERandom::NextInt(void)
	{
		int tmpState = _BLUERANDOM_A * (m_state % _BLUERANDOM_Q) - _BLUERANDOM_R * (m_state / _BLUERANDOM_Q);
		(tmpState > 0) ? m_state = tmpState : m_state = tmpState + _BLUERANDOM_M;
		return m_state;
	}
	int CBLUERandom::NextInt(const int nLow, const int nHigh) {return nLow + NextInt() % (nHigh - nLow);}
	int CBLUERandom::NextInt(const int n) {return NextInt(0, n);}
	double CBLUERandom::NextDouble() {return (double)NextInt() / _BLUERANDOM_M;}

private:
	void Init(int initialVal)
	{
		if(initialVal < 0)
			initialVal += _BLUERANDOM_M;
		m_state = initialVal;
		if(m_state==0)
			m_state=1;
	}

private :
	int m_state;
};

#endif
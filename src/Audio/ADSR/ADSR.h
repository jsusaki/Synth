//
//  ADRS.h
//
//  Created by Nigel Redmon on 12/18/12.
//  EarLevel Engineering: earlevel.com
//  Copyright 2012 Nigel Redmon
//
//  For a complete explanation of the ADSR envelope generator and code,
//  read the series of articles by the author, starting here:
//  http://www.earlevel.com/main/2013/06/01/envelope-generators/
//
//  License:
//
//  This source code is provided as is, without warranty.
//  You may copy and distribute verbatim copies of this document.
//  You may modify and use this source code to create binary code for your own purposes, free or commercial.
//
//  1.01  2016-01-02  njr   added calcCoef to SetTargetRatio functions that were in the ADSR widget but missing in this code
//  1.02  2017-01-04  njr   in calcCoef, checked for rate 0, to support non-IEEE compliant compilers
//  1.03  2020-04-08  njr   changed float to double; large target ratio and rate resulted in exp returning 1 in calcCoef
//

#ifndef ADRS_h
#define ADRS_h

class ADSR 
{
public:
	ADSR();
	~ADSR();
public:
    double Process();
    double GetOutput();
    int GetState();
	void Gate(int on);
    void SetAttackRate(double rate);
    void SetDecayRate(double rate);
    void SetReleaseRate(double rate);
	void SetSustainLevel(double level);
    void SetTargetRatioA(double targetRatio);
    void SetTargetRatioDR(double targetRatio);
    void Reset();

    enum envState 
    {
        env_idle = 0,
        env_attack,
        env_decay,
        env_sustain,
        env_release
    };

protected:
	int state;
	double output;
	double attackRate;
	double decayRate;
	double releaseRate;
	double attackCoef;
	double decayCoef;
	double releaseCoef;
	double sustainLevel;
    double targetRatioA;
    double targetRatioDR;
    double attackBase;
    double decayBase;
    double releaseBase;
 
    double CalcCoef(double rate, double targetRatio);
};

inline double ADSR::Process() 
{
	switch (state) 
    {
        case env_idle:
            break;
        case env_attack:
            output = attackBase + output * attackCoef;
            if (output >= 1.0) 
            {
                output = 1.0;
                state = env_decay;
            }
            break;
        case env_decay:
            output = decayBase + output * decayCoef;
            if (output <= sustainLevel) 
            {
                output = sustainLevel;
                state = env_sustain;
            }
            break;
        case env_sustain:
            break;
        case env_release:
            output = releaseBase + output * releaseCoef;
            if (output <= 0.0) 
            {
                output = 0.0;
                state = env_idle;
            }
	}
	return output;
}

inline void ADSR::Gate(int gate) 
{
	if (gate)
		state = env_attack;
	else if (state != env_idle)
        state = env_release;
}

inline int ADSR::GetState() 
{
    return state;
}

inline void ADSR::Reset() 
{
    state = env_idle;
    output = 0.0;
}

inline double ADSR::GetOutput() 
{
	return output;
}

#endif

#ifndef IEFFECT_HPP
#define IEFFECT_HPP

class IEffect {
public:
    virtual ~IEffect() {}

    /**
     * @param buffer: Ses örneklerini (samples) içeren dizi. (-1.0 ile 1.0 arası değerler)
     * @param frameCount: Bu döngüde kaç adet ses örneği işleneceği.
     */
    virtual void process(float* buffer, unsigned long frameCount) = 0;
};

#endif 

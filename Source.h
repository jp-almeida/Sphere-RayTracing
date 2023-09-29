#ifndef _SOURCE_H
#define _SOURCE_H

class Source{
  public:
    Source();
    virtual Vect getLightPosition() { return Vect(0.0f, 0.0f, 0.0f); }
    virtual Color getLightColor() { return Color(1.0f, 1.0f, 1.0f, 0.0f); }
};

Source::Source(){}
#endif
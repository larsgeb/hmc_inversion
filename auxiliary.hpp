//
// Created by Lars Gebraad on 7/10/17.
// Remake of the original aux.cpp, with a bit more logic to it.
//

#ifndef HMC_VSP_AUXILIARY_HPP
#define HMC_VSP_AUXILIARY_HPP

class prior;

class data;

class posterior;

class forwardModel;

//class taylorExpansion;

class prior {
public:
    // Fields
    unsigned long _numberParameters;
    std::vector<double> _mean;
    std::vector<double> _std;
    std::vector<std::vector<double> > _massMatrix;

    // Constructors and destructor
    prior();

    prior(std::vector<double> mean, std::vector<double> std);

    ~prior();

    // Member functions
    double misfit(std::vector<double> q);

private:
    void setMassMatrix();
//    void readPrior(const char *filename);
};

class data {
public:
    data(int numberData);

    data();

    int _numberData;
    std::vector<double> _observedData;
    std::vector<std::vector<double> > _inverseCD;
    std::vector<std::vector<double> > _misfitMatrix; // This is the forward model transposed times inverseCD times forward
    // model. Pre-calculated for speed.

    void setICDMatrix(double std);

    void readData(const char *filename);

    void writeData(const char *filename);

    double misfit(std::vector<double> in_parameters, forwardModel m);

    void setMisfitMatrix(std::vector<std::vector<double>> designMatrix);
};

class posterior {
public:
    double misfit(std::vector<double> parameters, prior &in_prior, data &in_data, forwardModel m);
};

class forwardModel {
public:
    // Constructors & destructors
    forwardModel(int numberParameters);

    forwardModel();

    // Fields
    int _numberParameters;
    std::vector<std::vector<double>> _designMatrix;

    // Methods
    void constructDesignMatrix(int numberParameters);

    std::vector<double> calculateData(std::vector<double> parameters);
};

/*class taylorExpansion {
public:
    taylorExpansion();

    // Constructor and destructor
    taylorExpansion(std::vector<double> parameters, double stepRatio, prior &in_prior, data &in_data, posterior
    &in_posterior);

    ~taylorExpansion();

    // Fields
    double _stepRatio;
    std::vector<double> _expansionPoint;
    double _functionValue;
    std::vector<double> _firstDerivativeValue;
    std::vector<std::vector<double> > _secondDerivativeValue;
    prior _prior;
    data _data;
    posterior _posterior;

    // Member functions
    std::vector<double> gradient(std::vector<double> q);

    void updateExpansion(std::vector<double> in_expansionPoint);

private:
    void calculate0(std::vector<double> expansionPoint);

    void calculate1(std::vector<double> expansionPoint);

    void calculate2(std::vector<double> expansionPoint);
};*/

#endif //HMC_VSP_AUXILIARY_HPP

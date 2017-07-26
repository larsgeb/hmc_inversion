//
// Created by Lars Gebraad on 7/11/17.
//

#include <time.h>
#include <vector>
#include <math.h>
#include "auxiliary.hpp"
#include "montecarlo.hpp"
#include <stdlib.h>
#include "randomnumbers.hpp"
#include "linearalgebra.hpp"
#include <stdio.h>
#include <iostream>

montecarlo::montecarlo(prior &in_prior, data &in_data, posterior &in_posterior, forwardModel
in_model, int in_nt, double in_dt, int in_iterations) {
    _prior = in_prior;
    _data = in_data;
    _model = in_model;
    _posterior = in_posterior;
    _nt = in_nt;
    _dt = in_dt;
    _iterations = in_iterations;

    /* Initialise random number generator. ----------------------------------------*/
    srand((unsigned int) time(0));

    _currentModel = _prior._mean;
    _proposedModel = _prior._mean;

    // These shouldn't be initialized yet, but better safe than sorry when using std::vector.push_back().
    _proposedMomentum.clear();
    _currentMomentum.clear();

    // To ensure equal oscillations
    _massMatrix = MatrixMatrixProduct(TransposeMatrix(_model._designMatrix), _model._designMatrix);
    _massMatrix = VectorToDiagonal(MatrixTrace(_massMatrix)); // Take only diagonal entries


    // Assigning a random moment to the momentum vectors
    for (int i = 0; i < _prior._numberParameters; i++) {
        // But this momentum assignment is as of yet only the diagonal.
        _proposedMomentum.push_back(randn(0.0, sqrt(_massMatrix[i][i])));
        _proposedModel[i] = randn(_prior._mean[i], _prior._std[i]);
        _currentModel[i] = _proposedModel[i];
        _currentMomentum.push_back(_proposedMomentum[i]);
    }
}

montecarlo::~montecarlo() {

}

/* Propose test model based on prior. ---------------------------------------------*/
void montecarlo::propose_metropolis() {
    for (int i = 0; i < _prior._numberParameters; i++)
        _proposedModel[i] = randn(_prior._mean[i], _prior._std[i]);
}

/* Proposal based on the solution of Hamilton's equation. -------------------------*/
void montecarlo::propose_hamilton() {
    /* Draw random prior momenta. */
    for (int i = 0; i < _prior._numberParameters; i++) {
//        _proposedMomentum[i] = 0;
        _proposedMomentum[i] = randn(0.0, sqrt(_massMatrix[i][i])); // only diagonal implemented!
    }
    /* Integrate Hamilton's equations. */

    // TODO only call this for last iterations, to save a lot of time
    FILE *trajectoryfile;
    trajectoryfile = fopen("OUTPUT/trajectory.txt", "w");
    leap_frog(trajectoryfile);
    fclose(trajectoryfile);
}

double montecarlo::chi() {
    return _posterior.misfit(_proposedModel, _prior, _data, _model);
}

/* Misfit for Hamiltonian Monte Carlo. --------------------------------------------*/
double montecarlo::energy() {
    double H = chi();
    for (int i = 0; i < _prior._numberParameters; i++) {
        H += 0.5 * _proposedMomentum[i] * _proposedMomentum[i] / _massMatrix[i][i];
    }
    return H;
}

void montecarlo::sample(bool hamilton) {
    double x = (hamilton ? energy() : chi()); // We evaluate th complete hamiltonian
    double x_new;
    int accepted = 0;

    FILE *samplesfile;
    samplesfile = fopen("OUTPUT/samples.txt", "w");
    write_sample(samplesfile, x, 0);
    for (int it = 1; it < _iterations; it++) {
        hamilton ? propose_hamilton() : propose_metropolis();
        x_new = (hamilton ? energy() : chi());

        double result;
        result = x - x_new;
        double result_exponent;
        result_exponent = exp(result);

        if ((x_new < x) || (result_exponent > randf(0.0, 1.0))) {
            accepted++;
            x = x_new;
            _currentModel = _proposedModel;
            write_sample(samplesfile, x, it);
//            std::cout << "model " << it + 1 << " is accepted" << std::endl;
        }
//        std::cout << "iteration " << it + 1 << " energy " << x_new << std::endl;
    }
    fprintf(samplesfile, "%i ", accepted);
    fprintf(samplesfile, "\n");
    fclose(samplesfile);
    std::cout << "Number of accepted models: " << accepted;
}

/* Leap-frog integration of Hamilton's equations. ---------------------------------*/
void montecarlo::leap_frog(_IO_FILE *trajectoryfile) {

    _proposedModel = _currentModel;
    _currentMomentum  =_proposedMomentum;

    std::vector<double> misfitGrad;
    double angle1, angle2;
    /* March forward. -------------------------------------------------------------*/
    for (int it = 0; it < _nt; it++) {
//        Use this to validate that Hamiltonian is preserved (hint, for stepsize 0.05 there's already a large round-off error
//        std::cout << energy() << std::endl;

        /* First half step in momentum. */
        misfitGrad = _posterior.gradientMisfit(_proposedModel, _prior, _data);
        for (int i = 0; i < _prior._numberParameters; i++) {
            _proposedMomentum[i] = _proposedMomentum[i] - 0.5 * _dt * misfitGrad[i];
        }
        misfitGrad.clear();

        // TODO only call this for last iterations, to save a lot of time
        write_trajectory(trajectoryfile, it);
        /* Full step in position. */
        for (int i = 0; i < _prior._numberParameters; i++) {
            _proposedModel[i] = _proposedModel[i] + _dt * _proposedMomentum[i] / (_massMatrix[i][i]);
        }

        misfitGrad = _posterior.gradientMisfit(_proposedModel, _prior, _data);
        for (int i = 0; i < _prior._numberParameters; i++) {
            _proposedMomentum[i] = _proposedMomentum[i] - 0.5 * _dt * misfitGrad[i];
        }
        misfitGrad.clear();

        /* Check no-U-turn criterion. */
        angle1 = VectorVectorProduct(_proposedMomentum, VectorDifference(_currentModel,_proposedModel));
        angle2 = VectorVectorProduct(_currentMomentum, VectorDifference(_proposedModel,_currentModel));

        if (angle1 > 0.0 && angle2 > 0.0) {
//            std::cout << "U-Turn!" << std::endl;
            break;
        }
    }

}

void montecarlo::write_sample(FILE *pfile, double misfit, int iteration) {
    if (iteration == 0) fprintf(pfile, "%d %d\n", (int) _prior._numberParameters, _iterations);

    for (int i = 0; i < (int) _prior._numberParameters; i++) fprintf(pfile, "%lg ", _proposedModel[i]);
    fprintf(pfile, "%lg ", misfit);
    fprintf(pfile, "\n");
}

void montecarlo::write_trajectory(FILE *pfile, int iteration) {
    if (iteration == 0) fprintf(pfile, "%d %i\n", (int) _prior._numberParameters, _nt);

    for (int i = 0; i < (int) _prior._numberParameters; i++) fprintf(pfile, "%.20lg ", _proposedModel[i]);
    fprintf(pfile, "%.20lg ", _posterior.misfit(_proposedModel, _prior, _data, _model));
    fprintf(pfile, "\n");

}
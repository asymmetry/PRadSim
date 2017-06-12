//
// qesed.cc
// Quasi-Elastic Scattering of Electrons on Deutron
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "TROOT.h"
#include "TError.h"
#include "TObject.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH2D.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TRandom2.h"
#include "TVector3.h"

#include "Math/Functor.h"
#include "Math/GSLIntegrator.h"

#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <iostream>

#define IntOpt ROOT::Math::IntegrationOneDim::kADAPTIVE
#define IntTol 0.001

#define InterpolPoints 1001
#define InterpolType ROOT::Math::Interpolation::kCSPLINE

#define Abs   TMath::Abs
#define Exp   TMath::Exp
#define Log   TMath::Log
#define DiLog TMath::DiLog
#define Sqrt  TMath::Sqrt
#define Sin   TMath::Sin
#define Cos   TMath::Cos
#define Tan   TMath::Tan
#define ASin  TMath::ASin
#define ACos  TMath::ACos
#define ATan  TMath::ATan
#define ATan2 TMath::ATan2

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const double pi = TMath::Pi();
const double deg = pi / 180.0;
const double me= 0.510998928e-3;
const double me2 = TMath::Power(me, 2);
const double mp = 0.9382720813;
const double mp2 = TMath::Power(mp, 2);
const double mn = 0.9395654133;
const double mn2 = TMath::Power(mn, 2);
const double md = 1.875612928;
const double md2 = TMath::Power(md, 2);
const double alpha = 1.0 / 137.036;
const double e = Sqrt(4.0 * pi * alpha);
const double mup = 2.79284736;
const double mun = -1.91304272; 
const double fm = 0.197327;
const double mkb = 389.379404;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

inline double Pow2(double arg)
{
    return TMath::Power(arg, 2);
}

inline double Pow3(double arg)
{
    return TMath::Power(arg, 3);
}

inline double Pow4(double arg)
{
    return TMath::Power(arg, 4);
}

inline double Pow5(double arg)
{
    return TMath::Power(arg, 5);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double ei_e;

double theta_e_min, theta_e_max;

double ef_e, theta_e, phi_e;
double ef_p, theta_p, phi_p;
double ef_n, theta_n, phi_n;

double omega;

TLorentzVector vi_e, vi_d;
TLorentzVector vf_e, vf_p, vf_n;

double p_fermi, theta_fermi;

TRandom2 *PseRan = new TRandom2();

double proton_xs_sin(double theta);

ROOT::Math::Functor1D func_proton_xs_sin(&proton_xs_sin);
ROOT::Math::GSLIntegrator integrator_proton_xs_sin(IntOpt);

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double GEp(double q2)
{
    const double a11 = 2.90966;
    const double a12 = -1.11542229;
    const double a13 = 3.866171e-2;
    const double b11 = 14.5187212;
    const double b12 = 40.88333;
    const double b13 = 99.999998;
    const double b14 = 4.579e-5;
    const double b15 = 10.3580447;

    double t = q2 / (4.0 * mp2); // tau

    return (1.0 + a11 * t + a12 * Pow2(t) + a13 * Pow3(t)) / (1.0 + b11 * t + b12 * Pow2(t) + b13 * Pow3(t) + b14 * Pow4(t) + b15 * Pow5(t));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double GMp(double q2)
{
    const double a21 = -1.43573;
    const double a22 = 1.19052066;
    const double a23 = 2.5455841e-1;
    const double b21 = 9.70703681;
    const double b22 = 3.7357e-4;
    const double b23 = 6.0e-8;
    const double b24 = 9.9527277;
    const double b25 = 12.7977739;

    double t = q2 / (4.0 * mp2); // tau

    return mup * (1.0 + a21 * t + a22 * Pow2(t) + a23 * Pow3(t)) / (1.0 + b21 * t + b22 * Pow2(t) + b23 * Pow3(t)) + b24 * Pow4(t) + b25 * Pow5(t);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double GEn(double q2)
{
    const double A = 1.70;
    const double B = 3.30;
    //const double eta = 5.6;

    double GD = 1.0 / Pow2(1.0 + q2 / 0.71);

    double t = q2 / (4.0 * mn2); // tau

    //return mun * t * GD / (1.0 + eta * t); // Galster parameterization
    return A * t * GD / (1.0 + B * t); // Kelly fit
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double GMn(double q2)
{
    // Kelly fit
    const double a41 = 2.33;
    const double b41 = 14.72;
    const double b42 = 24.20;
    const double b43 = 84.1;

    double t = q2 / (4.0 * mn2); // tau

    return mun * (1.0 + a41 * t) / (1.0 + b41 * t + b42 * Pow2(t) + b43 * Pow3(t));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double epelasticxs(double ei, double theta) 
{
    // the lepton mass isn't neglected here, see arXiv:1401.2959
    double ef = ((ei + mp) * (mp * ei + me2) + Sqrt(mp2 - Pow2(me * Sin(theta))) * (Pow2(ei) - me2) * Cos(theta)) / (Pow2(ei + mp) - (Pow2(ei) - me2) * Pow2(Cos(theta)));

    double q2 = 4.0 * ei * ef * Sin(theta / 2.0) * Sin(theta / 2.0);
    double tau = q2 / (4.0 * mp2); // tau
    //double eps = 1.0 / (1.0 + 2.0 * (1.0 + tau) * Pow2(Tan(theta / 2.0))); // epsilon
    double myeps = 1.0 / (1.0 - 2.0 * (1.0 + tau) * (-q2 + 2.0 * me2) / (4.0 * ei * ef - q2)); // modified epsilon
    double d = (ef / ei) * Sqrt((Pow2(ei) - me2) / (Pow2(ef) - me2));

    return Pow2(alpha / (2.0 * ei)) * ((1.0 - q2 / (4.0 * ei * ef)) / Pow2(q2 / (4.0 * ei * ef))) / d * (mp * (Pow2(ef) - me2) / (mp * ei * ef + me2 * (ef - ei - mp))) / (myeps * (1.0 + tau)) * (myeps * Pow2(GEp(q2)) + tau * Pow2(GMp(q2)));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double enelasticxs(double ei, double theta) 
{
    // the lepton mass isn't neglected here, see arXiv:1401.2959
    double ef = ((ei + mn) * (mn * ei + me2) + Sqrt(mn2 - Pow2(me * Sin(theta))) * (Pow2(ei) - me2) * Cos(theta)) / (Pow2(ei + mn) - (Pow2(ei) - me2) * Pow2(Cos(theta)));

    double q2 = 4.0 * ei * ef * Sin(theta / 2.0) * Sin(theta / 2.0);
    double tau = q2 / (4.0 * mn2); // tau
    //double eps = 1.0 / (1.0 + 2.0 * (1.0 + tau) * Pow2(Tan(theta / 2.0))); // epsilon
    double myeps = 1.0 / (1.0 - 2.0 * (1.0 + tau) * (-q2 + 2.0 * me2) / (4.0 * ei * ef - q2)); // modified epsilon
    double d = (ef / ei) * Sqrt((Pow2(ei) - me2) / (Pow2(ef) - me2));
    
    return Pow2(alpha / (2.0 * ei)) * ((1.0 - q2 / (4.0 * ei * ef)) / Pow2(q2 / (4.0 * ei * ef))) / d * (mn * (Pow2(ef) - me2) / (mn * ei * ef + me2 * (ef - ei - mn))) / (myeps * (1.0 + tau)) * (myeps * Pow2(GEn(q2)) + tau * Pow2(GMn(q2)));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double hulthen(double p_fermi)
{
    const double A2 = 2.088e-3;
    const double B2 = 6.76e-2;

    double pf2 =  p_fermi * p_fermi;

    return pf2 * TMath::Power(((pf2 + A2) * (pf2 + B2)), -2.0);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double calc_rest_frame_energy(double mmn)
{
    // calculate energy of electron in nucleon rest frame

    double e_n = Sqrt(mmn * mmn + p_fermi * p_fermi);

    return (1.0 / mmn) * ei_e * (e_n - p_fermi * Cos(theta_fermi));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double calc_theta_rf(double theta_lab, double mmn)
{
    double ei_e_rf = calc_rest_frame_energy(mmn);
    double e_t = Sqrt(mmn * mmn + p_fermi * p_fermi);

    double arg = 1 - (1 - Cos(theta_lab)) / (e_t - p_fermi * Cos(theta_lab - theta_fermi)) * (mmn * ei_e / ei_e_rf);

    if (arg <= 1 && arg >= -1)
        return acos(arg);

    return 0.0;
}

double calc_theta_lab(double theta_rf, double mmn)
{
    const double epsilon_min = 0.001;

    double theta_min = 0;
    double theta_max = pi;
    double theta_mid = 0.5 * (theta_min + theta_max);
    double epsilon = calc_theta_rf(theta_mid, mmn) - theta_rf;

    int iter = 0;
    while (Abs(epsilon) > epsilon_min) {
        theta_mid = 0.5 * (theta_min + theta_max);
        epsilon = calc_theta_rf(theta_mid, mmn) - theta_rf;
        if (epsilon < 0) {
            theta_min = theta_mid;
        } else if (epsilon > 0) {
            theta_max = theta_mid;
        }
        iter++;
        if (iter > 500) {
            std::cout << "couldn't find solution at: p_fermi = " << p_fermi << ", theta_fermi = " << theta_fermi << ", theta_rf = " << theta_rf << std::endl;
            return theta_rf;
        }
    }

    return theta_mid;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Double_t proton_theta_rf(Double_t *x, Double_t *par)
{
    return calc_theta_rf(x[0], mp);
}

double proton_jacobian(double theta)
{
    static TF1 *f_theta_rf = new TF1("f_theta_rf", proton_theta_rf, 0, pi, 0);

    if (p_fermi == 0.0) return 1.0;

    double theta_lab = calc_theta_lab(theta, mp);

    //return Sin(theta) / Sin(theta_lab) * f_theta_rf->Derivative(theta_lab);
    return f_theta_rf->Derivative(theta_lab);
}

double proton_xs_sin(double theta)
{
    double ei_e_rf = calc_rest_frame_energy(mp);

    return epelasticxs(ei_e_rf, theta) * proton_jacobian(theta) * Sin(theta);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void fill_weight_table_proton(TH2D *weight_table_proton)
{
    TAxis *x_axis = weight_table_proton->GetXaxis();
    TAxis *y_axis = weight_table_proton->GetYaxis();

    for(int i = 1; i <= weight_table_proton->GetNbinsX(); i++) {
        for(int j = 1; j <= weight_table_proton->GetNbinsY(); j++) {
            if (((i - 1) * 100 + j) % 1000 == 0) std::cout << ((i - 1) * 100) + j << std::endl;
            
            double cos_theta_fermi = x_axis->GetBinCenter(i);
            p_fermi = y_axis->GetBinCenter(j);

            theta_fermi = acos(cos_theta_fermi);
            //double ei_e_rf = calc_rest_frame_energy(mp);

            double theta_min = calc_theta_rf(theta_e_min, mp);
            double theta_max = calc_theta_rf(theta_e_max, mp);
            
            double proton_xs = integrator_proton_xs_sin.Integral(theta_min, theta_max);
            double hulthen_weight = hulthen(p_fermi);

            weight_table_proton->SetBinContent(i, j, hulthen_weight * proton_xs);
        }
    }
}

void fill_xs_table_proton(TH2D *xs_table_proton)
{
    TAxis *x_axis = xs_table_proton->GetXaxis();
    TAxis *y_axis = xs_table_proton->GetYaxis();
    
    for (int i = 1; i <= xs_table_proton->GetNbinsX(); i++) {
        for (int j = 1; j <= xs_table_proton->GetNbinsY(); j++) {
            double e = x_axis->GetBinCenter(i);
            double t = y_axis->GetBinCenter(j);
            xs_table_proton->SetBinContent(i, j, epelasticxs(e, t) * mkb);
        }
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

int main()
{
    char mychar[64];
/*
    std::cout << "Full energy of the incident lepton (MeV): " << std::flush;
    std::cin.getline(mychar, 64);
    ei_e = 0.001 * atof(mychar); // MeV

    std::cout << "Minimum polar angle of the electron (degree): " << std::flush;
    std::cin.getline(mychar, 64);
    theta_e_min = atof(mychar) * deg;

    std::cout << "Maximum polar angle of the electron (degree): " << std::flush;
    std::cin.getline(mychar, 64);
    theta_e_max = atof(mychar) * deg;

    std::cout << "Number of events to generate: " << std::flush;
    std::cin.getline(mychar, 64);
    int nevents = atoi(mychar);
*/
    ei_e = 1.1;
    theta_e_min = 1.9999 * deg;
    theta_e_max = 2.0001 * deg;
    int nevents = 10000;

    PseRan->SetSeed(0);

    omega = 2.0 * pi * (Cos(theta_e_min) - Cos(theta_e_max));

    vi_e.SetPxPyPzE(0.0, 0.0, Sqrt(Pow2(ei_e) - me2), ei_e);
    vi_d.SetPxPyPzE(0.0, 0.0, 0.0, md);

    TH2D *weight_table_proton = new TH2D("weight_table_proton", "weight_table_proton", 100, -1.0, 1.0, 100, 0.0, 0.6);
    //TH2D *weight_table_neutron = new TH2D("weight_table_neutron",
    //"weight_table_neutron", 100, -1.0, 1.0, 100, 0.0, 0.6);

    integrator_proton_xs_sin.SetFunction(func_proton_xs_sin);
    integrator_proton_xs_sin.SetRelTolerance(IntTol);

    fill_weight_table_proton(weight_table_proton);

    p_fermi = 0.7;
    theta_fermi = pi;
    double ei_e_rf_max = calc_rest_frame_energy(mp);
    theta_fermi = 0;
    double ei_e_rf_min = calc_rest_frame_energy(mp);

    TH2D *xs_table_proton = new TH2D("xs_table_proton", "xs_table_proton", 100, ei_e_rf_min, ei_e_rf_max, 1000, 0.2 * deg, 8.2 * deg);

    fill_xs_table_proton(xs_table_proton);

    FILE *fp = fopen("edquasiel.dat", "w");
    
    for (int i = 0; i < nevents; ++i) {
        if (i % 1000 == 0 && i != 0) std::cout << i << std::endl;

        double cos_theta_fermi = 0.0;
        weight_table_proton->GetRandom2(cos_theta_fermi, p_fermi);
        double sin_theta_fermi = Sqrt(1 - cos_theta_fermi * cos_theta_fermi);
        theta_fermi = ACos(cos_theta_fermi);

        double phi_fermi = 2 * pi * PseRan->Rndm();
        TVector3 v_fermi3(p_fermi * sin_theta_fermi * Cos(phi_fermi), p_fermi * sin_theta_fermi * Sin(phi_fermi), p_fermi * cos_theta_fermi);

        double ei_e_rf = calc_rest_frame_energy(mp);
        std::cout << ei_e_rf << std::endl;
        double theta_e_rf_min = calc_theta_rf(theta_e_min, mp);
        double theta_e_rf_max = calc_theta_rf(theta_e_max, mp);

        std::cout << theta_e_rf_min * 180.0 / pi << " " << theta_e_rf_max * 180.0 / pi << std::endl;
        if (ei_e_rf > ei_e_rf_max) ei_e_rf = ei_e_rf_max;

        TAxis *x_axis = xs_table_proton->GetXaxis();
        int ebin = x_axis->FindBin(ei_e_rf);
        std::cout << ebin << std::endl;
        TH1D *y_proj = xs_table_proton->ProjectionY("", ebin, ebin);

        double theta_e_rf = y_proj->GetRandom();
        
        while (theta_e_rf < theta_e_rf_min || theta_e_rf > theta_e_rf_max) theta_e_rf = y_proj->GetRandom();

        std::cout << theta_e_rf * 180.0 / pi << std::endl;

        theta_e = calc_theta_lab(theta_e_rf, mp);

        ef_e = ((ei_e + mp) * (mp * ei_e + me2) + Sqrt(mp2 - Pow2(me * Sin(theta_e))) * (Pow2(ei_e) - me2) * Cos(theta_e)) / (Pow2(ei_e + mp) - (Pow2(ei_e) - me2) * Pow2(Cos(theta_e))); // first guess, elastic

        double ef_n = Sqrt(mn2 + p_fermi * p_fermi);

        double old_ef_e = 0.0;
        TVector3 vf_p3;

        for (int j = 0; j < 1000; j++) {
            vf_e.SetPxPyPzE(Sqrt(Pow2(ef_e) - me2) * Sin(theta_e), 0.0, Sqrt(Pow2(ef_e) - me2) * Cos(theta_e), ef_e);

            vf_p3 = vi_e.Vect() + v_fermi3 - vf_e.Vect();
            ef_p = Sqrt(vf_p3 * vf_p3 + mp2);

            old_ef_e = ef_e;

            ef_e = ei_e + md - ef_p - ef_n;

            std::cout << ef_e << std::endl;

            if (Abs(old_ef_e - ef_e) < 0.0001) break;
        }

        vf_p.SetVect(vf_p3);
        vf_p.SetE(ef_p);

        vf_n = vi_e + vi_d - vf_e - vf_p;

        ef_e = vf_e.E();
        theta_e = vf_e.Theta();
        phi_e = vf_e.Phi();

        ef_p = vf_p.E();
        theta_p = vf_p.Theta();
        phi_p = vf_p.Phi();

        ef_n = vf_n.E();
        theta_n = vf_n.Theta();
        phi_n = vf_n.Phi();

        fprintf(fp, "%9.3lf %8.6lf %8.5lf %9.3lf %8.6lf %8.5lf %9.3lf %8.6lf %8.5lf\n", 1000.0 * ef_e, theta_e, phi_e, 1000.0 * ef_p , theta_p, phi_p, 1000.0 * ef_n, theta_n, phi_n);
        printf("%9.3lf %8.6lf %8.5lf %9.3lf %8.6lf %8.5lf %9.3lf %8.6lf %8.5lf\n", 1000.0 * ef_e, theta_e, phi_e, 1000.0 * ef_p , theta_p, phi_p, 1000.0 * ef_n, theta_n, phi_n);
    }
    
    fclose(fp);
/*
    fp = fopen("moller.info", "w");

    fprintf(fp, "cross section (averaged over the solid angle):\n");
    fprintf(fp, "%lf microbarn / steradian\n", xsint / omega);
    fprintf(fp, "integrated luminosity:\n");
    fprintf(fp, "%lf inverse microbarn\n", nevents / xsint);

    printf("cross section (averaged over the solid angle):\n");
    printf("%lf microbarn / steradian\n", xsint / omega);
    printf("integrated luminosity:\n");
    printf("%lf inverse microbarn\n", nevents / xsint);

    fclose(fp);
*/
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

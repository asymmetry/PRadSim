//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// HyCalParameterisation.cc
// Developer : Chao Peng, Chao Gu
// History:
//   Aug 2012, C. Peng, Original version.
//   Jan 2017, C. Gu, Rewrite with ROOT support.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "HyCalParameterisation.hh"

#include "ConfigObject.h"
#include "ConfigParser.h"

#include "G4Box.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VPVParameterisation.hh"

#include "G4ios.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"

#include <string>
#include <unordered_map>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HyCalParameterisation::HyCalParameterisation(const std::string &path, const std::string &type) : G4VPVParameterisation(), ConfigObject(), fType(type)
{
    if (!path.empty())
        Configure(path);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

HyCalParameterisation::~HyCalParameterisation()
{
    //
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void  HyCalParameterisation::Configure(const std::string &path)
{
    ConfigObject::Configure(path);

    LoadModuleList(GetConfig<std::string>("Module List"));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HyCalParameterisation::LoadModuleList(const std::string &path)
{
    if (path.empty())
        return;

    ConfigParser c_parser;

    if (!c_parser.ReadFile(path)) {
        G4cout << "ERROR: failed to read module list file " << "\"" << path << "\"" << G4endl;
        return;
    }

    std::string name, type;
    double sizex, sizey, length, x, y, z;
    HyCal_Module_Type t;

    // some info that is not read from list
    while (c_parser.ParseLine()) {
        if (!c_parser.CheckElements(8))
            continue;

        c_parser >> name >> type >> sizex >> sizey >> length >> x >> y >> z;

        bool rotate = false;

        if (type.compare("PbGlass") == 0) {
            t = Lead_Glass;

            if ((x < 353.09 && y > 352.75) || (x > -353.09 && y < -352.75))
                rotate = true;
        } else if (type.compare("PbWO4") == 0)
            t = Lead_Tungstate;
        else
            continue;

        sizex = sizex * mm;
        sizey = sizey * mm;
        length = length * mm;
        x = x * mm;
        y = y * mm;
        z = 0.0 * mm;

        if (type == fType)
            moduleList.push_back(HyCal_Module(name, t, sizex, sizey, length, -1.0 * x, y, z, rotate));
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void HyCalParameterisation::ComputeTransformation(const G4int copyNo, G4VPhysicalVolume *physVol) const
{
    if ((size_t)copyNo >= moduleList.size()) {
        G4cout << "ERROR: trying to load module no." << copyNo << ", but the loaded module list only has " << moduleList.size() << " modules" << G4endl;
        exit(1);
    }

    G4ThreeVector origin(moduleList[copyNo].x, moduleList[copyNo].y, moduleList[copyNo].z);
    physVol->SetTranslation(G4ThreeVector(origin));

    G4RotationMatrix *rm = new G4RotationMatrix();
    rm->rotateZ(-90.0 * deg);

    if (moduleList[copyNo].rot)
        physVol->SetRotation(rm);
    else
        physVol->SetRotation(0);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

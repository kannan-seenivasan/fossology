/*
 * Copyright (C) 2014-2015, Siemens AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <iostream>
#include <boost/tokenizer.hpp>
#include "ninkawrapper.hpp"
#include "utils.hpp"

string scanFileWithNinka(const State& state, const fo::File& file)
{
  FILE* in;
  char buffer[512];
  string command = "ninka " + file.getFileName();
  string result;

  if (!(in = popen(command.c_str(), "r")))
  {
    cout << "could not execute ninka command: " << command << endl;
    bail(1);
  }

  while (fgets(buffer, sizeof(buffer), in) != NULL)
  {
    result += buffer;
  }

  if (pclose(in) != 0)
  {
    cout << "could not execute ninka command: " << command << endl;
    bail(1);
  }

  return result;
}

vector<string> extractLicensesFromNinkaResult(string ninkaResult)
{
  string licensePart = extractLicensePartFromNinkaResult(ninkaResult);
  return splitLicensePart(licensePart);
}

// Ninka result format: filename;license1,license2,...,licenseN;details...
string extractLicensePartFromNinkaResult(string ninkaResult)
{
  string delimiters = ";\r\n";

  size_t first = ninkaResult.find_first_of(delimiters);
  size_t last = ninkaResult.find_first_of(delimiters, first + 1);

  return ninkaResult.substr(first + 1, last - 1 - first);
}

vector<string> splitLicensePart(string licensePart)
{
  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  boost::char_separator<char> separator(",");
  tokenizer tokens(licensePart, separator);

  vector<string> licenses;

  for (tokenizer::iterator iter = tokens.begin(); iter != tokens.end(); ++iter)
  {
    licenses.push_back(*iter);
  }

  return licenses;
}

vector<LicenseMatch> createMatches(vector<string> ninkaLicenseNames)
{
  vector<LicenseMatch> matches;
  for (vector<string>::const_iterator it = ninkaLicenseNames.begin(); it != ninkaLicenseNames.end(); ++it)
  {
    const string& ninkaLicenseName = *it;
    if(isLicenseCollection(ninkaLicenseName,matches))
    {
      continue;
    }    
    string fossologyLicenseName = mapLicenseFromNinkaToFossology(ninkaLicenseName);
    unsigned percentage = (ninkaLicenseName.compare("NONE") == 0 || ninkaLicenseName.compare("UNKNOWN") == 0) ? 0 : 100;
    LicenseMatch match = LicenseMatch(fossologyLicenseName, percentage);
    matches.push_back(match);
  }
  return matches;
}

string mapLicenseFromNinkaToFossology(string name)
{
  if (name.compare("NONE") == 0) return string("No_license_found");
  if (name.compare("UNKNOWN") == 0) return string("UnclassifiedLicense");
  if (name.compare("spdxMIT") ==0 ) return string("MIT");
  if (name.compare("Apachev1.0") == 0) return string("Apache-1.0");
  if (name.compare("Apachev2") == 0) return string("Apache-2.0");
  if (name.compare("GPLv1+") == 0) return string("GPL-1.0+");
  if (name.compare("GPLv2") == 0) return string("GPL-2.0");
  if (name.compare("GPLv2+") == 0) return string("GPL-2.0+");
  if (name.compare("GPLv3") == 0) return string("GPL-3.0");
  if (name.compare("GPLv3+") == 0) return string("GPL-3.0+");
  if (name.compare("LGPLv2") == 0) return string("LGPL-2.0");
  if (name.compare("LGPLv2+") == 0) return string("LGPL-2.0+");
  if (name.compare("LGPLv2_1") == 0
          || name.compare("LGPLv2.1") == 0) return string("LGPL-2.1");
  if (name.compare("LGPLv2_1+") == 0) return string("LGPL-2.1+");
  if (name.compare("LGPLv3") == 0) return string("LGPL-3.0");
  if (name.compare("LGPLv3+") == 0) return string("LGPL-3.0+");
  if (name.compare("GPLnoVersion") == 0) return string("GPL");
  if (name.compare("LesserGPLnoVersion") == 0
          || name.compare("LibraryGPLnoVersion")==0) return string("LGPL");
  if (name.compare("intelBSDLicense") == 0) return string("Intel-EULA");
  if (name.compare("spdxSleepyCat") == 0
          || name.compare("SleepyCat") == 0) return string("Sleepycat");
  if (name.compare("spdxBSD2") == 0
          || name.compare("BSD2") == 0) return string("BSD-2-Clause");
  if (name.compare("spdxBSD3") == 0
          || name.compare("BSD3") == 0) return string("BSD-3-Clause");
  if (name.compare("BSD3") == 0) return string("BSD-4-Clause");
  if (name.compare("spdxMIT") == 0) return string("MIT");
  if (name.compare("ZLIB") == 0) return string("Zlib");
  if (name.compare("openSSL") == 0
          || name.compare("openSSLvar1") ==0
          || name.compare("openSSLvar3") ==0) return string("OpenSSL");
  if (name.compare("QPLt") == 0) return string("QT(Commercial)");
  
  
  return name;
};

bool isLicenseCollection(string name, vector<LicenseMatch>& matches)
{
  if (name.compare("BisonException") == 0)
  {
    matches.push_back(LicenseMatch(string("GPL-2.0-with-bison-exception"), 50));
    matches.push_back(LicenseMatch(string("GPL-3.0-with-bison-exception"), 50));
    return true;
  }
  if (name.compare("spdxBSD4") == 0)
  {
    matches.push_back(LicenseMatch(string("BSD-4-Clause"), 50));
    matches.push_back(LicenseMatch(string("BSD-4-Clause-UC"), 50));
    return true;
  }
  if (name.compare("GPL2orBSD3") == 0)
  {
    matches.push_back(LicenseMatch(string("BSD-3-Clause"), 50));
    matches.push_back(LicenseMatch(string("GPL-2.0"), 50));
    return true;
  }
  if (name.compare("LGPLv2orv3") == 0)
  {
    matches.push_back(LicenseMatch(string("LGPL-2.0"), 50));
    matches.push_back(LicenseMatch(string("LGPL-3.0"), 50));
    return true;
  }
  if (name.compare("LGPLv2_1orv3") == 0)
  {
    matches.push_back(LicenseMatch(string("LGPL-2.1"), 50));
    matches.push_back(LicenseMatch(string("LGPL-3.0"), 50));
    return true;
  }
  if (name.compare("LGPLv2+MISTAKE") == 0)
  {
    matches.push_back(LicenseMatch(string("LGPL-2.1+"), 50));
    matches.push_back(LicenseMatch(string("LGPL-2.0+"), 50));
    return true;
  }
  if (name.compare("LGPLv2MISTAKE") == 0)
  {
    matches.push_back(LicenseMatch(string("LGPL-2.1"), 50));
    matches.push_back(LicenseMatch(string("LGPL-2.0"), 50));
    return true;
  }
  if (name.compare("GPLv1orArtistic") == 0)
  {
    matches.push_back(LicenseMatch(string("GPL-1.0"), 50));
    matches.push_back(LicenseMatch(string("Artistic-1.0"), 25));
    matches.push_back(LicenseMatch(string("Artistic-2.0"), 25));
    return true;
  }
  if (name.compare("GPL2orOpenIB") == 0)
  {
    matches.push_back(LicenseMatch(string("GPL-2.0"), 50));
    matches.push_back(LicenseMatch(string("BSD-2-Clause"), 50));
    return true;
  }
  if (name.compare("CDDLv1orGPLv2") == 0)
  {
    matches.push_back(LicenseMatch(string("CDDL-1.0"), 50));
    matches.push_back(LicenseMatch(string("GPL-2.0"), 50));
    return true;
  }
  if (name.compare("Apache-2orLGPLgeneric") == 0)
  {
    matches.push_back(LicenseMatch(string("Apache-2.0"), 50));
    matches.push_back(LicenseMatch(string("LGPL"), 50));
    return true;
  }
  
  return false;
}

/*
 * unmatched:
 * 
BSD2AdvInsteadOfBinary:BSDpre,BSDcondSource,BSDcondAdvRULE,BSDasIs,BSDWarr
BSD1:BSDpre,BSDcondBinary,BSDasIs,BSDWarr
BSDOnlyAdv:BSDpre,BSDcondAdvRULE,BSDasIs,BSDWarr
BSDOnlyEndorseNoWarranty:BSDpreLike,BSDcondEndorseRULE,BSDasIs
BSD2var1:BSDpre,BSDCondSourceVariant,BSDcondBinary,BSDasIs,BSDWarr
BSD2var2:BSDpre,BSDCondSourceVariant2,BSDcondBinary,BSDasIs,BSDWarr
BSD2aic700:BSDpre,BSDcondSource,BSDcondBinaryVar1,AsIsVariant2,LiabilityBSDVariantAIC700
BSD2SoftAndDoc:BSDpreSoftAndDoc,BSDcondSourceOrDoc,BSDcondBinary,BSDasIsSoftAndDoc,BSDWarr
BSDCairoStyleWarr:BSDpre,BSDcondSource,BSDcondBinary,BSDcondAdvPart2,OpenSSLwritCond,OpenSSLName,BSDasIs,BSDWarr
BSDdovecotStyle:BSDpre,BSDcondSource,BSDcondBinary,OpenSSLendorse,DovecotwriteCod,OpenSSLAckPart1,BSDcondAdvPart2,MITstyleCairoWarranty
ZLIBref:ZLibRef
boost-1:boostPermission,boostPreserve,boostAsIs,boostWarr
boost-1:boostRefv1
boost-1ref:boostSeev1
SSLeay:SSLCopy,SSLeayAttrib,SSLeayAdType,BSDpre,BSDcondSource,BSDcondBinary,BSDcondAdvRULE,SSLeayCrypto,SSLeayWindows,BSDasIs,BSDWarr,SSLeayCantChangeLic
 */
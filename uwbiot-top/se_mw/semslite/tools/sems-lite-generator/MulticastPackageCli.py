#! python3

# Copyright(C) NXP B.V. 2020
#
#  All rights are reserved. Reproduction in whole or in part is prohibited
#  without the prior written consent of the copy-right owner.
#  This source code and any compilation or derivative thereof is the sole
#  property of NXP B.V. and is provided pursuant to a Software License
#  Agreement. This code is the proprietary information of NXP B.V. and
#  is confidential in nature. Its use and dissemination by any party other
#  than NXP B.V. is strictly limited by the confidential information
#  provisions of the agreement referenced above.


import json
from typing import List
import argparse
import pathlib
from schema import MulticastPackage
from sems_lite_utils import SignatureParser
import sys

class Config:
    def __init__(self,
                 Copyright: str,
                 TargetCommercialName: str,
                 TargetEntityID: str,
                 Target12nc: str,
                 requiredFreeBytesNonVolatileMemory: int,
                 requiredFreeBytesTransientMemory: int,
                 MulticastPackageName: str,
                 MulticastPackageDescription: str,
                 SubComponentMetaData: List[MulticastPackage.SubComponentMetaDataType],
                 MulticastPackageVersion: str):
        self.Copyright = Copyright
        self.TargetCommercialName = TargetCommercialName
        self.Target12nc = Target12nc
        self.TargetEntityID = TargetEntityID
        self.requiredFreeBytesNonVolatileMemory = requiredFreeBytesNonVolatileMemory
        self.requiredFreeBytesTransientMemory = requiredFreeBytesTransientMemory
        self.MulticastPackageName = MulticastPackageName
        self.MulticastPackageDescription = MulticastPackageDescription
        self.SubComponentMetaData = SubComponentMetaData
        self.MulticastPackageVersion = MulticastPackageVersion

    @staticmethod
    def from_file(json_file_path):
        with open(json_file_path, 'r') as config_json_file:
            data = json.loads(config_json_file.read())
            obj = Config(**data)
            obj.SubComponentMetaData = list(map(MulticastPackage.SubComponentMetaDataType.from_json, data["SubComponentMetaData"]))
            return obj


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--config_file', required=True, nargs="?", help="Config File for MulticastPackage generation.")
    parser.add_argument('--script_file', required=True, nargs="?", help="Encrypted and Signed script as output by the ls-cgt tool.")
    parser.add_argument('--out', required=True, nargs="?", help="Output MulticastPackage json file.")
    args = parser.parse_args()

    config = Config.from_file(args.config_file)

    with open(args.script_file, 'r') as script_file:
        multicast_package = MulticastPackage.MulticastPackage.create(
            Copyright=config.Copyright,
            TargetCommercialName=config.TargetCommercialName,
            Target12nc=config.Target12nc,
            TargetEntityID=config.TargetEntityID,
            requiredFreeBytesNonVolatileMemory=config.requiredFreeBytesNonVolatileMemory,
            requiredFreeBytesTransientMemory=config.requiredFreeBytesTransientMemory,
            MulticastPackageName=config.MulticastPackageName,
            MulticastPackageDescription=config.MulticastPackageDescription,
            SubComponentMetaData=config.SubComponentMetaData,
            SignatureOverCommands=SignatureParser.SignatureParser.get_signature(args.script_file),
            MulticastCommands=script_file.read(),
            MulticastPackageVersion=config.MulticastPackageVersion
        )

    json_content = multicast_package.to_json()

    with open(args.out, 'w') as out_file:
        out_file.write(json_content)


if __name__ == "__main__":
    main()





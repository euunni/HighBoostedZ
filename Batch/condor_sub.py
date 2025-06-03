import os
import json
import argparse
from script_maker import create_jobs

def load_sample(era):
    list_path = f"./list/{era}.list"
    if not os.path.exists(list_path):
        print(f"[ERROR] Sample list file not found: {list_path}")
        return []
    with open(list_path) as f:
        samples = [line.strip() for line in f if line.strip()]
    return samples

def update_config(era, nfiles):
    config_path = f"./../input/config/{era}/config.json"
    
    if not os.path.exists(config_path):
        print(f"[ERROR] Config file not found: {config_path}")
        return False
    
    with open(config_path, 'r') as f:
        config = json.load(f)
    
    if "Processing" not in config:
        config["Processing"] = {}
    
    config["Processing"]["FilesPerJob"] = nfiles
    
    with open(config_path, 'w') as f:
        json.dump(config, f, indent=2)
    
    print(f"[INFO] Updated {config_path} with FilesPerJob={nfiles}")
    return True

def main():
    parser = argparse.ArgumentParser(description='Submit condor jobs for high boosted Z analysis')
    parser.add_argument('-n', '--nfiles', type=int, default=10,
                      help='Number of files per job (default: 10)')
    parser.add_argument('--eras', nargs='+', default=["2016_postVFP"],
                      help='Eras to process (default: 2016_postVFP)')
    
    args = parser.parse_args()
    
    print(f"[INFO] Will process {args.nfiles} files per job")
    
    for era in args.eras:
        if not update_config(era, args.nfiles):
            continue
        
        samples = load_sample(era)
        for sample in samples:
            list_dir = f"/u/user/haeun/CMSAnalysis/HighBoostedZ/Validation/HighBoostedZ/input/{era}/{sample}"
            create_jobs(era=era, sample=sample, list_dir=list_dir, files_per_job=args.nfiles)

if __name__ == "__main__":
    main()

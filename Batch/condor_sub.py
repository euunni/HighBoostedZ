import os
from script_maker import create_jobs

def load_sample(era):
    list_path = f"./list/{era}.list"
    if not os.path.exists(list_path):
        print(f"[ERROR] Sample list file not found: {list_path}")
        return []
    with open(list_path) as f:
        samples = [line.strip() for line in f if line.strip()]
    return samples

def main():
    eras = ["2016_postVFP", "2016_preVFP", "2017", "2018"]

    for era in eras:
        samples = load_sample(era)
        for sample in samples:
            list_dir = f"./../input/{era}/{sample}"
            create_jobs(era=era, sample=sample, list_dir=list_dir, files_per_job=50)

if __name__ == "__main__":
    main()

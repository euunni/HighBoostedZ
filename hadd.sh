#!/bin/bash

BASE_DIR="./output/250517"
ERAS=("2016_postVFP" "2016_preVFP" "2017" "2018")

for era in "${ERAS[@]}"; do
  echo "Processing era: $era"
  
  for sample_dir in "$BASE_DIR/$era"/*; do
    if [ ! -d "$sample_dir" ]; then
      continue
    fi
    
    sample=$(basename "$sample_dir")
    echo -e "Processing sample: $sample\n"
    
    combined_dir="$sample_dir/combined"
    mkdir -p "$combined_dir"
    
    pattern=""
    for root_file in "$sample_dir"/*.root; do
      if [ -f "$root_file" ]; then
        filename=$(basename "$root_file")
        pattern=$(echo "$filename" | sed 's/\(.*\)_[0-9]\+\.root/\1/')
        break
      fi
    done
    
    if [ -z "$pattern" ]; then
      echo -e "No root files found or pattern couldn't be determined.\n"
      continue
    fi
    
    hadd_command="hadd -f $combined_dir/${pattern}.root $sample_dir/${pattern}_*.root"
    echo "    $hadd_command"
    eval "$hadd_command"
  done
done

# Final combination
for era in "${ERAS[@]}"; do
  echo -e "\nProcessing final combination for era: $era\n"
  
  dy_files=""
  st_files=""
  tt_files=""
  wz_files=""
  data_files=""
  
  for sample_dir in "$BASE_DIR/$era"/*; do
    if [ ! -d "$sample_dir" ]; then
      continue
    fi
    
    sample=$(basename "$sample_dir")
    
    if [ ! -f "$sample_dir/combined"/*.root ]; then
      continue
    fi
    
    # Data
    if [[ "$sample" == *"Run"* ]]; then
      data_files="$data_files $sample_dir/combined/*.root"
    elif [[ "$sample" == DY* ]]; then
      # DY
      dy_files="$dy_files $sample_dir/combined/*.root"
    elif [[ "$sample" == ST* ]]; then
      # ST
      st_files="$st_files $sample_dir/combined/*.root"
    elif [[ "$sample" == TT* ]]; then
      # TT 
      tt_files="$tt_files $sample_dir/combined/*.root"
    elif [[ "$sample" == WJets* || "$sample" == WZ* || "$sample" == ZZ* ]]; then
      # WJets, WZ, ZZ 
      wz_files="$wz_files $sample_dir/combined/*.root"
    fi
  done
  
  final_dir="$BASE_DIR/$era/combined"
  mkdir -p "$final_dir"
  
  if [ ! -z "$dy_files" ]; then
    hadd_command="hadd -f $final_dir/DY_$era.root $dy_files"
    echo "  $hadd_command"
    eval "$hadd_command"
  fi
  
  if [ ! -z "$st_files" ]; then
    hadd_command="hadd -f $final_dir/ST_$era.root $st_files"
    echo "  $hadd_command"
    eval "$hadd_command"
  fi
  
  if [ ! -z "$tt_files" ]; then
    hadd_command="hadd -f $final_dir/TT_$era.root $tt_files"
    echo "  $hadd_command"
    eval "$hadd_command"
  fi
  
  if [ ! -z "$wz_files" ]; then
    hadd_command="hadd -f $final_dir/WZ_$era.root $wz_files"
    echo "  $hadd_command"
    eval "$hadd_command"
  fi
  
  if [ ! -z "$data_files" ]; then
    hadd_command="hadd -f $final_dir/Data_$era.root $data_files"
    echo "  $hadd_command"
    eval "$hadd_command"
  fi
done

echo -e "\nAll hadd operations completed!"

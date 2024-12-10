
item_type="PheneticHyb:float"
loc_file="./testlocs.txt" 
grid_file="${QHG4_DIR}/useful_stuff/gridpreparation/pilot_085.0_kya_256.qdf"
qdf_pat="ooa_pop-sapiens_SG_%06d.qdf"

if [ $# -gt 2 ]
then
    # must handle pattern specially
    qdf_dir=$1
    species=$2
    out_body=$3


    for param in $@
      do
      echo "checking param[${param}]"
      
      IFS='='
      read -ra args <<< "${param}"
      if [ "${args[0]}" == 'locs' ]
          then 
          loc_file=${args[1]}
      elif  [ "${args[0]}" == 'grid' ]
          then 
          grid_file=${args[1]}
      elif  [ "${args[0]}" == 'item' ]
          then 
          item_type=${args[1]}
      elif  [ "${args[0]}" == 'qdfp' ]
          then 
          qdf_pat=${args[1]}
      fi
      unset IFS
    done

    echo "### bighyb_overview ###"
    echo "qdf-dir:  ${qdf_dir}"
    echo "species:  ${species}"
    echo "out-body: ${out_body}"
    echo "grid:     ${grid_file}"
    echo "locs:     ${loc_file}"
    echo "item:     ${item_type}"
    echo "qdf-pat:  ${qdf_pat}"
    echo "###################3###"


    att_file="${out_body}_attrs.csv"
    hdf_file="${out_body}.hdf"
    csv_file="${out_body}.csv"
    
    rm  -f pure_atts*.csv
    rm  -f pure_arrs*.csv
    rm  -f ${out_body}*.csv
    rm  -f ${out_body}*.hdf
    rm  -f ${att_file}
    rm  -f ${hdf_file}
    rm  -f ${csv_file}

    res=0
    c=0  
    for f in ${qdf_dir}/*.qdf
      do
      
     
       ${QHG4_DIR}/useful_stuff/loopy.sh ${f} ${grid_file} ${item_type} ${out_body} ${loc_file} ${species} ${c} &

      c=`expr $c + 1`
    done

    wait

    echo "++++++++++++++++++ putting outbody csvs"
    cat ${out_body}_*.csv > ${out_body}.csv
    echo "++++++++++++++++++ putting atts csvs"
    cat pure_atts_*.csv  > pure_atts.csv
    echo "++++++++++++++++++ putting arrs csvs"
    cat pure_arrs_*.csv  > pure_arrs.csv

    sim_name=`echo ${qdf_dir} | gawk -F/ '{ print $NF }'`
    echo "qdf_dir [${qdf_dir}], sim_name [${sim_name}]"
    echo "++++++++++++++++++ /hdf_catenate ${out_body}.hdf  ${sim_name} ${out_body}_*.hdf"
    ${QHG4_DIR}/useful_stuff/hdf_catenate  ${out_body}.hdf ${sim_name}  ${out_body}_*.hdf

    rm pure_atts_*.csv pure_arrs_*.csv ${out_body}_*.csv  ${out_body}_*.hdf 
    
    if [ $res -eq 0 ]
    then
        echo "++++++++++++++++++ /column_merger.py -S';' ${out_body}.csv 1 pure_atts.csv 2- ${out_body}.csv 2-41 pure_arrs.csv all  ${att_file}"
        ${QHG4_DIR}/useful_stuff/column_merger.py -S';' ${out_body}.csv 1-2 pure_atts.csv 3- ${out_body}.csv 3- pure_arrs.csv all  ${att_file}
    else
        echo "had failure"
    fi

    rm pure_atts.csv pure_arrs.csv ${out_body}.csv

else
    echo "$0 - extracting hyb data from qdf files"
    echo "usage:"
    echo "  $0 <qdf-dir> <species> <out-body> <option>*"
    echo "where"
    echo "  qdf-dir     the directory containing the output qdfs"
    echo "  species     species name"
    echo "  out-body    output file name bidy (there will be a csv and a hdf)"
    echo "  option      expression <name>=<val>. possible names:"
    echo "              locs    location file    (default ${loc_file})"
    echo "              grid    grid file        (default ${grid_file})"
    echo "              qdfp    qdf file pattern (default ${qdf_pat})"
    echo "Example"
    echo "  $0 \"/home/jody/simmov/dummydumm/test_0*\" sapiens testout"
    echo "this will look at files of the form [${qdf_pat}] in all directories \"/home/jody/simmov/dummydumm/test_0*\""
    echo "The results will be written to testout_attrs.csv and testout.hdf"
  
fi

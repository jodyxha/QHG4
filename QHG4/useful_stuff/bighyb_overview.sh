echo "bighyb_overview.sh version date 14.12.2020"

mode="hyb"
item_type="PheneticHyb:float"
loc_file="./testlocs.txt" 
grid_file="${QHG4_DIR}/useful_stuff/gridpreparation/pilot_085.0_kya_256.qdf"
qdf_pat="ooa_pop-sapiens_SG_%06d.qdf"
mode="hyb"

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
      elif  [ "${args[0]}" == 'mode' ]
          then 
          mode=${args[1]}
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
    echo "mode:     ${mode}"
    echo "###################3###"


    att_file="${out_body}_attrs.csv"
    hdf_file="${out_body}.hdf"
    csv_file="${out_body}.csv"
    pure_att="pure_${out_body}_atts.csv"
    pure_arr="pure_${out_body}_arrs.csv"

    rm  -f ${pure_att}
    rm  -f ${pure_arr}
    rm  -f ${att_file}
    rm  -f ${hdf_file}
    rm  -f ${csv_file}

    # the scripts must be told whether to write a header or not 
    head="-h"

    res=0
    for f in ${qdf_dir}/*.qdf
      do

      if [ ${mode} == "hyb" ]
          then

          echo "before:"
          ls -l ${out_body}.csv

          echo "++++++++++++++++++ hyb_extract -p ${f} -g ${grid_file} -n ${item_type} -o ${out_body} -l ${loc_file}  ${head}"
          ${QHG4_DIR}/useful_stuff/hyb_extract -p ${f} -g ${grid_file} -n ${item_type} -o ${out_body} -l ${loc_file}  ${head}
          if [ $? -eq 0 ]
              then
              echo " --> ${out_body}.csv has `wc -l ${out_body}.csv | gawk '{ print $1 }'` lines"
              
          else 
              echo "break 1a"
              res=-1
              break
          fi
          echo "after:"
          ls -l ${out_body}.csv

      elif [ ${mode} == "ymt" ]
          then
          echo "++++++++++++++++++  ${QHG4_DIR}/useful_stuff/ymt_extract -p ${f} -g ${grid_file}  -o ${out_body} -l ${loc_file} "
          ${QHG4_DIR}/useful_stuff/ymt_extract -p ${f} -g ${grid_file} -o ${out_body} -l ${loc_file} 
          if [ $? -ne 0 ]
              then
              echo "break 1b"
              res=-1
              break
          fi
      else
          echo " unknown mode ${mode}"
          echo "break 1c"
          res=-1
      fi    
      
      echo "++++++++++++++++++ agentprops_overview.py ${f}  ${h} >> ${pure_att}" 
      ${QHG4_DIR}/useful_stuff/agentprops_overview.py   ${f}  ${head}  >> ${pure_att}
      if [ $? -eq 0 ]
      then
          echo " --> ${pure_att} has `wc -l ${pure_att} | gawk '{ print $1 }'` lines"
      else 
          echo "break 2"
          res=-1
          break
      fi
      h="v";
      
      echo "++++++++++++++++++ ArrivalCheck  -g ${f}:${species} -l  ${loc_file}  -C \"t\" ${head}  -f ${pure_arr}" 
      ${QHG4_DIR}/genes/ArrivalCheck  -g ${f}:${species} -l ${loc_file}  -C "t"  ${head}  -f ${pure_arr}
      if [ $? -eq 0 ]
      then
          echo " --> ${pure_arr} has `wc -l ${pure_arr} | gawk '{ print $1 }'` lines"
      else 
          echo "break 3"
          res=-1
          break
      fi
      # no more header after first iteration
      head=
    done

    if [ $res -eq 0 ]
    then
        if [ mode == "hyb" ]
            then
            echo "++++++++++++++++++  ${pure_att} (`wc -l ${pure_att} | gawk '{ print $1 }'` lines)"
            echo "++++++++++++++++++  ${out_body}.csv (`wc -l ${out_body}.csv | gawk '{ print $1 }'` lines)"
        fi
        echo "++++++++++++++++++  ${pure_arr} (`wc -l ${pure_arr} | gawk '{ print $1 }'` lines)"
        echo "++++++++++++++++++ /column_merger.py -S';' ${out_body}.csv 1-2 ${pure_att} 3- ${out_body}.csv 3- ${pure_arr} all  ${att_file}"
        if [ mode == "hyb" ]
            then
            ${QHG4_DIR}/useful_stuff/column_merger.py -S';' ${out_body}.csv 1-2 ${pure_att} 3- ${out_body}.csv 3- ${pure_arr} all  ${att_file}
        else 
            ${QHG4_DIR}/useful_stuff/column_merger.py -S';' ${pure_arr} all  ${att_file} 
        fi
    else
        echo "had failure"
    fi
else
    echo "$0 - extracting hyb or ymt data from qdf files"
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
    echo "              mode    'hyb' or 'ymt'   (default ${mode})"
    echo "Example"
    echo "  $0 \"/home/jody/simmov/dummydumm/test_0*\" sapiens testout"
    echo "this will look at files of the form [${qdf_pat}] in all directories \"/home/jody/simmov/dummydumm/test_0*\""
    echo "The results will be written to testout_attrs.csv and testout.hdf"
  
fi

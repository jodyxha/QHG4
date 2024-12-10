echo "creates one xml and one dat for neander and sapiens together"
if [ $# -gt 2 ]
then 
    echo "here i am" 
    csv_in=$1
    out_body=$2
    width=$3
    shift 3

    # dat and xml template files
    #xml_template_nea=/data/hyb_realistic/realistic_nea.xml
    #xml_template_sap=/data/hyb_realistic/realistic_sap.xml
    #dat_template_nea=${QHG4_DIR}/useful_stuff/realistic_nea.dat
    #dat_template_sap=${QHG4_DIR}/useful_stuff/realistic_sap_subsahara.dat
    #dat_template_nea=/data/hyb_realistic/realistic_nea.dat
    #dat_template_sap=/data/hyb_realistic/realistic_sap_subsahara.dat
    #xml_template_nea=/data/smallworld2/template_nea.xml
    #xml_template_sap=/data/smallworld2/template_sap.xml
    #dat_template_nea=/data/smallworld2/template_nea.dat
    #dat_template_sap=/data/smallworld2/template_sap.dat
    xml_template_nea=/data/smallworld2/template_nea_ymt.xml
    dat_template_nea=/data/smallworld2/template_nea.dat
    xml_template_sap=/data/smallworld2/template_sap_ymt.xml
    dat_template_sap=/data/smallworld2/template_sap.dat

    #out_dir="."
    out_dir="xmldat"
    xml_block=""

    parerr=0
    for param in $@
        do
        echo "checking param[${param}]"
        IFS='='
        read -ra args <<< "${param}"
        echo "arg: $args[0]"
        if [ "${args[0]}" == 'xml_tmpl_nea' ]
            then
            xml_template_nea=${args[1]}
        elif  [ "${args[0]}" == 'xml_tmpl_sap' ]
            then
            xml_template_sap=${args[1]}
        elif  [ "${args[0]}" == 'dat_tmpl_nea' ]
            then
            dat_template_nea=${args[1]}
        elif  [ "${args[0]}" == 'dat_tmpl_sap' ]
            then
            dat_template_sap=${args[1]}
        elif  [ "${args[0]}" == 'outsub' ]
            then
            outsub=${args[1]}
        elif  [ "${args[0]}" == 'xml_block' ]
            then
            xml_block=${args[1]}
        elif  [ "${args[0]}" == 'csv_ignore' ]
            then
            csv_ignore=${args[1]}
        else
            echo "unknown parameter: [${args[0]}]"
            parerr=1
        fi  
        unset IFS
	shift
    done

    if [ ${parerr} -eq 1 ]
    then
      echo
      echo "exiting script. Fix the bad parameter names"
      exit 255
    fi

    if [ "X${csv_ignore}" != "X" ]
    then
     	csv_ignore="-i ${csv_ignore}"
    fi

    if [ "X${xml_block}" != "X" ]
    then
     	xml_block="-b ${xml_block}"
    fi

    echo "templates used:"
    echo "  xml_template_nea=${xml_template_nea}"
    echo "  dat_template_nea=${dat_template_nea}"
    echo "  xml_template_sap=${xml_template_sap}"
    echo "  dat_template_sap=${dat_template_sap}"
    echo "ioutsub: ${outsub}"
    echo "xml_block: ${xml_block}"
    

    ll=`wc -l ${csv_in} | gawk '{ print $1 }'`
    echo "ll_ $ll"
    num_sims=`expr $ll \- 1`
    
    echo "there seem to be ${num_sims} simulations"

    num_sims=`expr ${num_sims} \- 1`

if [ "X${outsub}" == "X" ]
then

    jj=`echo ${csv_in} | awk -F_ '{ print $NF }' | sed 's/.csv//g'`

    out_dir2=${out_dir}/${out_dir}_${jj}
else
    out_dir2=${out_dir}/${outsub}
fi
    mkdir -p ${out_dir2}
    echo "created directory [${out_dir2}]"

    temp_dir=temp_${jj}
    mkdir -p ${temp_dir}
    echo "created directory [${temp_dir}]"
 
    # will return -1 if bad sep
    ${QHG4_DIR}/useful_stuff/csv2xmldat_obj.py  -w ${width} -x ${xml_template_nea} -d ${dat_template_nea} -c ${csv_in} -o ${temp_dir}/temp_${out_body}_nea ${xml_block} ${csv_ignore}
    if [ $? -eq 0 ]
    then
      # we dont't need to do the xml, because they would be the same we already have ; will return -1 if bad sep
      ${QHG4_DIR}/useful_stuff/csv2xmldat_obj.py  -w ${width} -a dat -x ${xml_template_sap} -d ${dat_template_sap} -c ${csv_in} -o ${temp_dir}/temp_${out_body}_sap ${xml_block} ${csv_ignore}
      if [ $? -eq 0 ]
      then
        for i in `seq 0 ${num_sims}`; 
        do 
          ii=`printf "%0${width}d" $i`;
          echo "cat ${temp_dir}/temp_${out_body}_sap_${ii}.dat > ${out_body}${ii}.dat"
          cat ${temp_dir}/temp_${out_body}_sap_${ii}.dat > ${out_body}_${ii}.dat
          echo "tail -n +2 ${temp_dir}/temp_${out_body}_nea_${ii}.dat >> ${out_body}${ii}.dat"
          tail -n +2 ${temp_dir}/temp_${out_body}_nea_${ii}.dat >> ${out_body}_${ii}.dat
          echo "moving [${out_body}_${ii}.dat] to  [${out_dir2}/${out_body}_${ii}.dat]"
          mv ${out_body}_${ii}.dat  ${out_dir2}/${out_body}_${ii}.dat
#          echo "moving [temp_${out_body}_sap_${ii}.xml] to  [${out_dir}/${out_body}${ii}_sap.xml]"
#          mv temp_${out_body}_nea_${ii}.xml  ${out_dir2}/${out_body}${ii}_sap.xml
          echo "moving [${temp_dir}/temp_${out_body}_nea_${ii}.xml] to  [${out_dir2}/${out_body}_${ii}]"
          mv ${temp_dir}/temp_${out_body}_nea_${ii}.xml  ${out_dir2}/${out_body}_${ii}.xml
        done
      else
        echo "cvs2xmldat had error with sap"
      fi
    else
        echo "cvs2xmldat had error with nea"
    fi
    
    rm -rf ${temp_dir}

else
  echo "$0 <csv_in> <out_body> <num_width>"
  echo "   xml_template_nea=<nea-xml-template-file>"
  echo "   xml_template_sap=<sap-xml-template-file>"
  echo "   dat_template_nea=<nea-dat-template-file>"
  echo "   dat_template_sap=<sap-dat-template-file>"
  echo "   outsub=<output_subdir>"
  echo "   xml_block=<param-name>[:<param-name>]*"
  echo "   csv-ignore=<param-name>[:<param-name>]*"
fi

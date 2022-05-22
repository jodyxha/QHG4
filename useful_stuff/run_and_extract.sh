echo "version date 14.12.2020"

t0=`date +%s`
echo "**** script $0 started at ${t0}"
# defualt values
start_time=-85000
num_iters=85000
shuffle="712371"
start_time=-85000
num_iters=85000
shuffle="712371"
out_period=10000
outputs="dat"
mode="hyb"
qhgmain="yes"
bighyb="yes"
tars="yes"
clean="yes" 

# give these variables reasonable values
data_dir=/home/jody/progs/multi_spc_QHG3_std/useful_stuff/gridpreparation/
grid_file=/home/jody/progs/multi_spc_QHG3_std/useful_stuff/gridpreparation/pilot_085.0_kya_256.qdf
loc_file=${QHG4_DIR}/genes/LocationListGrids_eurasafrplus.txt

if [ $# -gt 2 ]
then
    pop_in=$1
    out_top=$2
    out_prefix=$3
    shift 3
    parerr=0
    
    # see if we have some additional params
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
        elif  [ "${args[0]}" == 'iter' ]
        then 
            num_iters=${args[1]}
        elif  [ "${args[0]}" == 'start' ]
        then 
            start_time=${args[1]}
        elif  [ "${args[0]}" == 'shuffle' ]
        then 
            shuffle=${args[1]}
        elif  [ "${args[0]}" == 'period' ]
        then 
            out_period=${args[1]}
        elif  [ "${args[0]}" == 'outs' ]
        then 
            outputs=${args[1]}
        elif  [ "${args[0]}" == 'mode' ]
        then 
            if [ "${args[1]}" == 'ymt' ] ||  [ "${args[1]}" == 'hyb' ]
            then
                mode=${args[1]}
            fi
        elif  [ "${args[0]}" == 'qhgmain' ]
        then 
            qhgmain=${args[1]}
        elif  [ "${args[0]}" == 'bighyb' ]
        then 
            bighyb=${args[1]}
        elif  [ "${args[0]}" == 'tars' ]
        then 
            tars=${args[1]}
        elif  [ "${args[0]}" == 'clean' ]
        then 
            if [ "${args[1]}" == 'yes' ] ||  [ "${args[1]}" == 'no' ]
            then
                clean=${args[1]}
            fi
	else
	    echo "unknown parameter: [${args[0]}]"
	    parerr=1
        fi
        unset IFS
    done

    if [ ${parerr} -eq 1 ]
    then
      echo
      echo "exiting script. Fix the bad parameter names"
      exit 255
    fi

    IFS='+'
    read -ra out_modes <<< "${outputs}"
    unset IFS
    do_dat=0
    do_qdf=0
    do_att=0
    for i in "${out_modes[@]}"
    do
        if [ $i == "dat" ] 
        then
          do_dat=1
        elif [ $i == "qdf" ]
        then
          do_qdf=1
        elif [ $i == "att" ]
        then
          do_att=1
        else
          do_dat=1
        fi
    done
    
    echo "mode: ${mode}"
    echo "clean: ${clean}"
    echo "modes: ${outs}"
    echo "do_dat: $do_dat"
    echo "do_qdf: $do_qdf"
    echo "do_att: $do_att"

    mkdir -p ${out_top}

    echo "qhgmain is ${qhgmain}"
    if [ ${qhgmain} == "yes" ]
        then
:
        #u=`echo ${pop_in} | gawk -F_ '{ print $(NF-1) "_" $NF }'`
        #out_prefix=${out_prefix}_$u

        echo qdf
	echo "OMP_NUM_THREADS=16 ${QHG4_DIR}/app/QHGMain \\
            --data-dirs=${data_dir} \\
	    --events='write|grid+geo+pop:sapiens@${out_period},file|${QHG4_DIR}/useful_stuff/pilot_Tneg085.evt@[0]' \\
            --grid='${grid_file}' \\
            --log-file='${out_prefix}.log' \\
	    --output-dir='${out_top}/${out_prefix}' \\
            --output-prefix=ooa \\
            --pops='${pop_in}' \\
            --shuffle=${shuffle} \\
            --start-time='${start_time}'   \\
            --zip-output \\
            --num-iters='${num_iters}' > ${out_prefix}.out " 
	    
        OMP_NUM_THREADS=16 ${QHG4_DIR}/app/QHGMain \
            --data-dirs=${data_dir} \
            --events="write|grid+geo+pop:sapiens@${out_period},file|${QHG4_DIR}/useful_stuff/pilot_Tneg085.evt@[0]" \
            --grid="${grid_file}" \
            --log-file="${out_prefix}.log" \
            --output-dir="${out_top}/${out_prefix}" \
            --output-prefix=ooa \
            --pops="${pop_in}" \
            --shuffle=${shuffle} \
            --start-time="${start_time}" \
            --zip-output \
            --num-iters="${num_iters}" > ${out_prefix}.out

	if [ $? -eq 0 ]
        then
            echo "----- simulation done -----" >>  ${out_prefix}.out
        else
            echo "----- simulation failed ----" >>  ${out_prefix}.out
            echo "----- simulation failed ----"
            echo "exiting"
            exit
        fi


        echo "   " >>  ${out_prefix}.out
        echo "----- starting hyb extraction -----" >>  ${out_prefix}.out
        echo "gunzipping ${out_top}/${out_prefix}/*.gz" >>  ${out_prefix}.out
        gunzip ${out_top}/${out_prefix}/*.gz 
    fi
echo "bighyb is [${bighyb}]"   
    if [ ${bighyb} == "yes" ]
        then
        echo "running  ${QHG4_DIR}/useful_stuff/bigymt_overview.sh \ "  >> ${out_prefix}.out
        echo "        \"${out_top}/${out_prefix}\" \ "  >> ${out_prefix}.out
        echo "         sapiens ${out_prefix}   \ "  >> ${out_prefix}.out
        echo "         grid=${grid_file} "  >> ${out_prefix}.out
        echo "         locs=${loc_file}"  >> ${out_prefix}.out
        echo "         mode=${mode}"  >> ${out_prefix}.out 
   
        echo "${QHG4_DIR}/useful_stuff/bigymt_overview.sh \\
            "${out_top}/${out_prefix}" \\
            sapiens ${out_prefix}   \\
            grid=${grid_file} \\
            locs=${loc_file} \\
            mode=${mode} >> ${out_prefix}.out  2>> ${out_prefix}.out"

        ${QHG4_DIR}/useful_stuff/bighyb_overview.sh \
            "${out_top}/${out_prefix}" \
            sapiens ${out_prefix}   \
            grid=${grid_file} \
            locs=${loc_file} \
            mode=${mode} >> ${out_prefix}.out  2>> ${out_prefix}.out
        
	if [ $? -eq 0 ]
        then
            echo "----- bigymt_overview.sh completed -----" >>  ${out_prefix}.out
            echo "output files:"
            ls -l ${out_prefix}*           >>  ${out_prefix}.out
            ls -l pure*                    >>  ${out_prefix}.out
            ls -l ${out_top}/${out_prefix} >>  ${out_prefix}.out
        else
            echo "----- bighyb_overview.sh failed -----" >>  ${out_prefix}.out
            echo "----- bighyb_overview.sh failed -----" 
            echo "exiting"
            exit
        fi
    fi

    # tarball for qdfs if required
    if [ ${tars} == "yes" ]
        then
        if [ $do_qdf -ne 0 ]
            then
            echo "running tar zcvf ${out_prefix}_qdf.tar.gz  ${out_top}/${out_prefix}  >> ${out_prefix}.out"
            tar zcvf ${out_prefix}_qdf.tar.gz  ${out_top}/${out_prefix}  >> ${out_prefix}.out  2>> ${out_prefix}.out
        fi
        
        # tarball for inermediate csvs if required
        if [ $do_att -ne 0 ]
            then
            if [ ${mode} == "hyb" ]
                then
                echo "running tar zcvf ${out_prefix}_att.tar.gz   ${out_prefix}.csv pure_${out_prefix}_atts.csv pure_${out_prefix}_arrs.csv >> ${out_prefix}.out"
                tar zcvf ${out_prefix}_att.tar.gz   ${out_prefix}.csv pure_${out_prefix}_atts.csv pure_${out_prefix}_arrs.csv  >> ${out_prefix}.out  2>> ${out_prefix}.out
            else
                echo "running tar zcvf ${out_prefix}_att.tar.gz  pure_${out_prefix}_atts.csv pure_${out_prefix}_arrs.csv >> ${out_prefix}.out"
                tar zcvf ${out_prefix}_att.tar.gz  pure_${out_prefix}_atts.csv pure_${out_prefix}_arrs.csv  >> ${out_prefix}.out  2>> ${out_prefix}.out
            fi
        fi
        # tarball for hdf, out and final csv if required
        if [ $do_dat -ne 0 ]
            then
            echo "running tar zcvf ${out_prefix}_dat.tar.gz   ${out_prefix}.hdf ${out_prefix}_attrs.csv ${out_prefix}.out  >> ${out_prefix}.out"
            tar zcvf ${out_prefix}_dat.tar.gz ${out_prefix}.hdf ${out_prefix}_attrs.csv ${out_prefix}.out
        fi
    fi



    # clean intermediate files if required
    if [ "${clean}" == 'yes' ]
        then
        echo "cleaning intermediate files"
        for f in ${out_top}/${out_prefix}/
        do
            rm -rf ${f}
        done

        rm ${out_prefix}.hdf ${out_prefix}_attrs.csv ${out_prefix}.out ${out_prefix}.log ${out_prefix}.csv
        rm pure_${out_prefix}_atts.csv pure_${out_prefiy}_arrs.csv 
    fi

    t1=`date +%s`
    echo "**** script $0 finished at ${t1}"
    echo "**** total time: `expr ${t1} \- ${t0}`"
else
    echo "$0 - running a simulation and extracting hyb data"
    echo "usage:"
    echo "  $0 <pop-in> <out-top> <out-prefix> [grid=<grid-qdf>] [locs=<loc-file>]"
    echo "               [iter=<num_iters>] [shuffle=<seed>] [period=<out-per>]"
    echo "               [outs=<modes>]  [clean=<cleaning>]"
    echo "               [qhgmain=<yesno>] [bighyb=<yesno>] [tars=<yesno>]  [mode=<extrmode>]"
    echo "where"
    echo "  pop-in      comma-separated list of qdffiles or xml:dat files (as in the input foe QHGMain)"
    echo "  out-top     output superdirectory"
    echo "  out-prefix  prefix for output files (is also the name of the output subdirectory in out-top)"
    echo "  grid-qdf    qdf file containing grid group (default: ${grid_file})"
    echo "  loc-file    path to location file (default: ${loc_file})"
    echo "  num-iters   number of iterations (default: ${num_iters})"
    echo "  seed        random seed (default: ${shuffle})"
    echo "  out_per     output periodicity (default: ${out_period})"
    echo "  modes       output mode (default: ${outputs})"
    echo "              modes ::= <mode>['+'<modes>}"
    echo "              mode  ::= 'dat' | 'qdf' | 'att'"
    echo "  qhgmain     run QHGMain ('yes' or 'no')"
    echo "  bighyb      run bighyb_overview ('yes' or 'no')"
    echo "  tars        tar outpout files ('yes' or 'no')"
    echo "  extrmode    extraction type ('hyb' or 'ymt')"
    echo "  cleaning    if 'yes', intermediate files will be removed (default: ${clean})"
fi

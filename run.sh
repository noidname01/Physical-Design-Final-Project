make
for i in {1..4}
do
    rm ./inputs/case$i/case$i"_bot"*
    rm ./inputs/case$i/case$i"_top"*
    mkdir -p ./outputs/case$i
    mkdir -p ./log/case$i
    start=$(($(date +%s%N)/1000000))
    ./bin/place ./inputs/case$i/case$i | tee ./log/case$i/case$i"_part_log"
    ./bin/ntuplace-r -aux ./inputs/case$i/case$i"_top.aux" -out ./inputs/case$i/case$i"_top" -nolegal -nodetail | tee ./log/case$i/case$i"_pl_top_log"
    ./bin/ntuplace-r -aux ./inputs/case$i/case$i"_bot.aux" -out ./inputs/case$i/case$i"_bot" -nolegal -nodetail| tee ./log/case$i/case$i"_pl_bot_log"
    ./bin/place ./inputs/case$i/case$i | tee ./log/case$i/case$i"_via_log"
    ./bin/ntuplace-r -aux ./inputs/case$i/case$i"_top_term.aux" -out ./outputs/case$i/case$i"_top_term" | tee ./log/case$i/case$i"_pl_top_term_log"
    ./bin/ntuplace-r -aux ./inputs/case$i/case$i"_bot_term.aux" -out ./outputs/case$i/case$i"_bot_term" | tee ./log/case$i/case$i"_pl_bot_term_log"
    end=$(($(date +%s%N)/1000000))
    echo "Running Time:" $((end-start))"ms"
done

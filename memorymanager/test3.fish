for prob in 1 10 100 500
  for structure in 'flexible' 'segmentTree'  'avlBlock' # 'avlInt'
    for compression in 'simd' 'nocomp' 'lz4' 'lz4hc' 'simd' 'snappy' 'simple9'
      set filename "./data_space/massif-14-$structure-$compression-$prob"
      rm -f $filename
      echo $filename
      valgrind --tool=massif --massif-out-file="$filename"  ./experiment-time.out $structure $compression  9 14 30 $prob 0 2>/dev/null
    end
  end
end

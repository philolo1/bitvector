for doRandom in '0' '1'
  for structure in 'avlInt' 'avlBlock' 'segmentTree' 'flexible'
    for compression in 'nocomp' 'lz4' 'lz4hc' 'simd' 'snappy' 'simple9'
      for prob in 1 10 100 500
        set filename "./data/$structure-$compression-$prob-$doRandom"
        rm -f $filename
        echo $filename
        for rep in (seq 8)
          ./experiment-time.out $structure $compression  9 16 20 $prob $doRandom >> $filename
        end
      end
    end
  end
end

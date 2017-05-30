(module
  (import "js" "print" (func $print (param f64)))
  (global $PI f64 (f64.const 3.141592653589793))
  (global $SOLAR_MASS f64 (f64.const 39.47841760435743))
  (global $DAYS_PER_YEAR f64 (f64.const 365.24))
  ;; (global $size (mut i32) (i32.const 0))
  (memory $mem 1)

  (func $createBody (param i32 f64 f64 f64 f64 f64 f64 f64)
    (local $offset i32)

    get_local 0
    i32.const 56
    i32.mul
    set_local $offset
    get_local $offset
    get_local 1
    f64.store
    get_local $offset
    get_local 2
    f64.store offset=8
    get_local $offset
    get_local 3
    f64.store offset=16
    get_local $offset
    get_local 4
    f64.store offset=24
    get_local $offset
    get_local 5
    f64.store offset=32
    get_local $offset
    get_local 6
    f64.store offset=40
    get_local $offset
    get_local 7
    f64.store offset=48)

  (func $createNBodySystem (param $size i32)
    (local $px f64)
    (local $py f64)
    (local $pz f64)
    (local $m f64)
    (local $i i32)

    ;; Sun
    i32.const 0
    f64.const 0
    f64.const 0
    f64.const 0
    f64.const 0
    f64.const 0
    f64.const 0
    get_global $SOLAR_MASS
    call $createBody
    ;; Jupiter
    i32.const 1
    f64.const 4.84143144246472090e+00
    f64.const -1.16032004402742839e+00
    f64.const -1.03622044471123109e-01
    f64.const 1.66007664274403694e-03 get_global $DAYS_PER_YEAR f64.mul
    f64.const 7.69901118419740425e-03 get_global $DAYS_PER_YEAR f64.mul
    f64.const -6.90460016972063023e-05 get_global $DAYS_PER_YEAR f64.mul
    f64.const 9.54791938424326609e-04 get_global $SOLAR_MASS f64.mul
    call $createBody
    ;; Saturn
    i32.const 2
    f64.const 8.34336671824457987e+00
    f64.const 4.12479856412430479e+00
    f64.const -4.03523417114321381e-01
    f64.const -2.76742510726862411e-03 get_global $DAYS_PER_YEAR f64.mul
    f64.const 4.99852801234917238e-03 get_global $DAYS_PER_YEAR f64.mul
    f64.const 2.30417297573763929e-05 get_global $DAYS_PER_YEAR f64.mul
    f64.const 2.85885980666130812e-04 get_global $SOLAR_MASS f64.mul
    call $createBody
    ;; Uranus
    i32.const 3
    f64.const 1.28943695621391310e+01
    f64.const -1.51111514016986312e+01
    f64.const -2.23307578892655734e-01
    f64.const 2.96460137564761618e-03 get_global $DAYS_PER_YEAR f64.mul
    f64.const 2.37847173959480950e-03 get_global $DAYS_PER_YEAR f64.mul
    f64.const -2.96589568540237556e-05 get_global $DAYS_PER_YEAR f64.mul
    f64.const 4.36624404335156298e-05 get_global $SOLAR_MASS f64.mul
    call $createBody
    ;; Neptune
    i32.const 4
    f64.const 1.53796971148509165e+01
    f64.const -2.59193146099879641e+01
    f64.const 1.79258772950371181e-01
    f64.const 2.68067772490389322e-03 get_global $DAYS_PER_YEAR f64.mul
    f64.const 1.62824170038242295e-03 get_global $DAYS_PER_YEAR f64.mul
    f64.const -9.51592254519715870e-05 get_global $DAYS_PER_YEAR f64.mul
    f64.const 5.15138902046611451e-05 get_global $SOLAR_MASS f64.mul
    call $createBody

    f64.const 0.0 set_local $px
    f64.const 0.0 set_local $py
    f64.const 0.0 set_local $pz
    get_local $size
    i32.const 56
    i32.mul
    set_local $size

    i32.const 0 set_local $i
    block $1 loop $2
      get_local $i
      get_local $size
      i32.eq
      br_if $1

      get_local $i f64.load offset=48 set_local $m
      get_local $i f64.load offset=24
      get_local $m
      f64.mul
      get_local $px
      f64.add
      set_local $px

      get_local $i f64.load offset=48 set_local $m
      get_local $i f64.load offset=32
      get_local $m
      f64.mul
      get_local $py
      f64.add
      set_local $py

      get_local $i f64.load offset=48 set_local $m
      get_local $i f64.load offset=40
      get_local $m
      f64.mul
      get_local $pz
      f64.add
      set_local $pz
      
      get_local $i
      i32.const 56
      i32.add
      set_local $i
      br $2
    end end
    
    i32.const 0 get_local $px get_local $py get_local $pz call $body_offsetMomentum)

  (func $bodySystem_advance (param $size i32) (param $dt f64)
    (local $dx f64)
    (local $dy f64)
    (local $dz f64)
    (local $distance f64)
    (local $mag f64)
    (local $i i32)
    (local $j i32)
    (local $mi f64)
    (local $mj f64)
    (local $distance2 f64)
    get_local $size
    i32.const 56
    i32.mul
    tee_local $size
    set_local $i
    block $1 loop $2
      get_local $i
      i32.eqz
      br_if $1
      get_local $i
      i32.const 56
      i32.sub
      tee_local $i
      set_local $j
      block $3 loop $4
        get_local $j
        i32.eqz
        br_if $3
        get_local $j
        i32.const 56
        i32.sub
        set_local $j
        get_local $i f64.load
        get_local $j f64.load
        f64.sub
        tee_local $dx
        get_local $dx
        f64.mul
        get_local $i f64.load offset=8
        get_local $j f64.load offset=8
        f64.sub
        tee_local $dy
        get_local $dy
        f64.mul
        f64.add
        get_local $i f64.load offset=16
        get_local $j f64.load offset=16
        f64.sub
        tee_local $dz
        get_local $dz
        f64.mul
        f64.add
        tee_local $distance2
        f64.sqrt
        set_local $distance

        get_local $dt
        get_local $distance get_local $distance2 f64.mul
        ;; get_local $distance get_local $distance get_local $distance f64.mul f64.mul
        f64.div
        tee_local $mag
        get_local $i f64.load offset=48
        f64.mul
        set_local $mi
        get_local $j f64.load offset=48
        get_local $mag
        f64.mul
        set_local $mj

        get_local $i
        get_local $i f64.load offset=24
        get_local $dx
        get_local $mj
        f64.mul
        f64.sub
        f64.store offset=24

        get_local $i
        get_local $i f64.load offset=32
        get_local $dy
        get_local $mj
        f64.mul
        f64.sub
        f64.store offset=32

        get_local $i
        get_local $i f64.load offset=40
        get_local $dz
        get_local $mj
        f64.mul
        f64.sub
        f64.store offset=40

        get_local $j
        get_local $j f64.load offset=24
        get_local $dx
        get_local $mi
        f64.mul
        f64.add
        f64.store offset=24

        get_local $j
        get_local $j f64.load offset=32
        get_local $dy
        get_local $mi
        f64.mul
        f64.add
        f64.store offset=32

        get_local $j
        get_local $j f64.load offset=40
        get_local $dz
        get_local $mi
        f64.mul
        f64.add
        f64.store offset=40

        br $4
      end end
      br $2
    end end

    get_local $size set_local $i
    block $1 loop $2
      get_local $i
      i32.eqz
      br_if $1
      get_local $i
      i32.const 56
      i32.sub
      set_local $i
      get_local $i
      get_local $i f64.load
      get_local $dt
      get_local $i f64.load offset=24
      f64.mul
      f64.add
      f64.store

      get_local $i
      get_local $i f64.load offset=8
      get_local $dt
      get_local $i f64.load offset=32
      f64.mul
      f64.add
      f64.store offset=8

      get_local $i
      get_local $i f64.load offset=16
      get_local $dt
      get_local $i f64.load offset=40
      f64.mul
      f64.add
      f64.store offset=16
    
      br $2
    end end
    )
  
  (func $bodySystem_energy (param $size i32) (result f64)
    (local $dx f64)
    (local $dy f64)
    (local $dz f64)
    (local $distance f64)
    (local $e f64)
    (local $i i32)
    (local $j i32)
    f64.const 0.0 set_local $e
    get_local $size
    i32.const 56
    i32.mul
    set_local $size

    block $1 loop $2
      get_local $i
      get_local $size
      i32.eq
      br_if $1

      get_local $i f64.load offset=24
      get_local $i f64.load offset=24
      f64.mul
      get_local $i f64.load offset=32
      get_local $i f64.load offset=32
      f64.mul
      get_local $i f64.load offset=40
      get_local $i f64.load offset=40
      f64.mul
      f64.add
      f64.add
      get_local $i f64.load offset=48
      f64.mul
      f64.const 0.5
      f64.mul
      get_local $e
      f64.add
      set_local $e

      get_local $i
      i32.const 56
      i32.add
      set_local $j

      block $3 loop $4
        get_local $j
        get_local $size
        i32.eq
        br_if $3

        get_local $i f64.load
        get_local $j f64.load
        f64.sub
        set_local $dx
        
        get_local $i f64.load offset=8
        get_local $j f64.load offset=8
        f64.sub
        set_local $dy

        get_local $i f64.load offset=16
        get_local $j f64.load offset=16
        f64.sub
        set_local $dz

        get_local $dx get_local $dx f64.mul
        get_local $dy get_local $dy f64.mul
        get_local $dz get_local $dz f64.mul
        f64.add
        f64.add
        f64.sqrt
        set_local $distance

        get_local $e
        get_local $i f64.load offset=48
        get_local $j f64.load offset=48
        f64.mul
        get_local $distance
        f64.div
        f64.sub
        set_local $e

        get_local $j
        i32.const 56
        i32.add
        set_local $j
        br $4
      end end

      get_local $i
      i32.const 56
      i32.add
      set_local $i
      br $2
    end end

    get_local $e)

  (func $body_offsetMomentum (param $i i32) (param f64 f64 f64)
    (local $offset i32)
    get_local $i
    i32.const 56
    i32.mul
    set_local $offset

    get_local $offset
    get_local 1
    f64.neg
    get_global $SOLAR_MASS
    f64.div
    f64.store offset=24

    get_local $offset
    get_local 2
    f64.neg
    get_global $SOLAR_MASS
    f64.div
    f64.store offset=32

    get_local $offset
    get_local 3
    f64.neg
    get_global $SOLAR_MASS
    f64.div
    f64.store offset=40
    )


  (func $main
    (local $i i32)
    (local $n i32)
    i32.const 0 set_local $i
    i32.const 50000000 set_local $n

    i32.const 5 call $createNBodySystem
    i32.const 5 call $bodySystem_energy call $print
    block $exit
      loop $cont
        get_local $i
        get_local $n
        i32.eq
        br_if $exit
        i32.const 5
        f64.const 0.01
        call $bodySystem_advance
        get_local $i
        i32.const 1
        i32.add
        set_local $i
        br $cont
      end
    end
    i32.const 5 call $bodySystem_energy call $print)

  (export "main" (func $main))
)

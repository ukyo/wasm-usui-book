const mem = new WebAssembly.Memory({ initial: 256 });
let f64arr = new Float64Array(mem.buffer);
let i32arr = new Int32Array(mem.buffer);

// see http://indiegamr.com/generate-repeatable-random-numbers-in-js/
// the initial seed
Math.seed = 6;

// in order to work 'Math.seed' must NOT be undefined,
// so in any case, you HAVE to provide a Math.seed
Math.seededRandom = (max, min) => {
  max = max || 1;
  min = min || 0;

  Math.seed = (Math.seed * 9301 + 49297) % 233280;
  const rnd = Math.seed / 233280;

  return min + rnd * (max - min);
};

function randomizeArray(arr, max, min) {
  Math.seed = 6;
  arr.forEach((x, i) => arr[i] = Math.seededRandom(max, min));
}

const randomizeF64Array = () => randomizeArray(f64arr, 1, -1);
const randomizeI32Array = () => randomizeArray(i32arr, 100000000,-100000000);

function test(arr) {
  for (let i = 0, n = arr.length - 1; i < n; ++i) {
    if (arr[i] > arr[i + 1]) {
      console.log("fail");
      return;
    }
  }
  console.log("ok");
}

function med3(x, y, z) {
  if (x < y) {
    if (y < z) return y; else if (z < x) return x; else return z;
  } else {
    if (z < y) return y; else if (x < z) return x; else return z;
  }
}

function _qsort_64(left, right) {
  if (left < right) {
    let i = left, j = right, tmp, pivot = med3(f64arr[i], f64arr[j], f64arr[i + ((j - i) >>> 1)]);
    while (1) {
      while (f64arr[i] < pivot) i++;
      while (pivot < f64arr[j]) j--;
      if (i >= j) break;
      tmp = f64arr[i]; f64arr[i] = f64arr[j]; f64arr[j] = tmp;
      i++; j--;
    }
    _qsort_64(left, i - 1);
    _qsort_64(j + 1, right);
  }
}

function qsort_f64(start, end) {
  return _qsort_64(start, end - 1);
}

function _qsort_i32(left, right) {
  if (left < right) {
    let i = left, j = right, tmp, pivot = med3(i32arr[i], i32arr[j], i32arr[i + ((j - i) >>> 1)]);
    while (1) {
      while (i32arr[i] < pivot) i++;
      while (pivot < i32arr[j]) j--;
      if (i >= j) break;
      tmp = i32arr[i]; i32arr[i] = i32arr[j]; i32arr[j] = tmp;
      i++; j--;
    }
    _qsort_i32(left, i - 1);
    _qsort_i32(j + 1, right);
  }
}

function qsort_i32(start, end) {
  return _qsort_i32(start, end - 1);
}

fetch('qsort.wasm').then(res => res.arrayBuffer()).then(buffer => {
  return WebAssembly.instantiate(buffer, {
    js: { mem }
  });
}).then(({ module, instance }) => {
  [100, 100000, undefined].forEach(x => {
    f64arr = new Float64Array(mem.buffer, 0, x);
    i32arr = new Int32Array(mem.buffer, 0, x);

    new Benchmark.Suite()
    .add(`[js qsort_f64 ${f64arr.length}items]`, () => qsort_f64(0, f64arr.length))
    .add(`[wasm qsort_f64 ${f64arr.length}items]`, () => instance.exports.qsort_f64(0, f64arr.length))
    .on('start', randomizeF64Array)
    .on('cycle', event => {
      result.appendChild(document.createTextNode(String(event.target) + '\n'));
    })
    .run();
    result.appendChild(document.createTextNode('--------------------------------\n'));
    new Benchmark.Suite()
    .add(`[js qsort_i32 ${i32arr.length}items]`, () => qsort_i32(0, i32arr.length))
    .add(`[wasm qsort_i32 ${i32arr.length}items]`, () => instance.exports.qsort_i32(0, i32arr.length))
    .on('start', randomizeI32Array)
    .on('cycle', event => {
      result.appendChild(document.createTextNode(String(event.target) + '\n'));
    })
    .run();
    result.appendChild(document.createTextNode('--------------------------------\n'));
  });
});

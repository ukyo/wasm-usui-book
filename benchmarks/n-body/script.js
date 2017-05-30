fetch("./n-body.wasm")
.then(res => res.arrayBuffer())
.then(buff => WebAssembly.instantiate(buff, {
  js: {
    print: x => console.log(x.toFixed(9))
  }
}))
.then(({module, instance}) => {
  var start = performance.now();
  instance.exports.main();
  console.log("[wasm]: ", performance.now() - start);
});

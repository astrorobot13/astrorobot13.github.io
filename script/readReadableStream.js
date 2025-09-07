async function readReadableStream(stream, blobOptions={}) {
  var reader = stream.getReader(), data = { value: undefined, done: false }, blobParts = [];
  while (!data.done) {
    data = await reader.read();
    if (!data.done) {
      blobParts.push(data.value);
    }
  }
  return new Blob(blobParts, blobOptions);
}
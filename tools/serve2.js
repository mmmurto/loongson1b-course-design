const root = "C:/Users/mmirz/fs/tftp";
Bun.serve({
  port: 8080,
  hostname: "0.0.0.0",
  async fetch(req) {
    const name = decodeURIComponent(new URL(req.url).pathname.slice(1));
    if (!name) return new Response("vmlinux rootfs.ext2\n");
    const f = Bun.file(root + "/" + name);
    if (await f.exists()) {
      console.log(`[${new Date().toLocaleTimeString()}] GET ${name} (${f.size} bytes)`);
      return new Response(f, { headers: { "Content-Type": "application/octet-stream", "Content-Length": String(f.size) } });
    }
    return new Response("404", { status: 404 });
  },
});
console.log("http file server on 8080 serving " + root);

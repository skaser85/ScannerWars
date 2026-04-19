let SHOW_MENU = true;

const SCREEN_WIDTH = 1920;
const SCREEN_HEIGHT = 1080;

const IMAGE_DIM = 75;

const BOX_VELOCITY = 20;

const BUTTON_FONT_SIZE = 24;
const BUTTON_COLOR = "white";
const BUTTON_HOVER_COLOR = "aqua";
const BUTTON_PAD_X = 20;
const BUTTON_PAD_Y = 10;


const { Engine, World, Bodies, Body, Runner, Composite, Events } = Matter;

// engine + world
const engine = Engine.create();
engine.world.gravity.y = 0;        // no gravity
engine.world.gravity.x = 0;
engine.world.gravity.scale = 0;
engine.positionIterations = 10;
engine.velocityIteratrions = 10;

// boundaries (static)
const thickness = 10;
const walls = [
  Bodies.rectangle(SCREEN_WIDTH/2, -thickness/2, SCREEN_WIDTH, thickness, { isStatic: true }),             // top
  Bodies.rectangle(SCREEN_WIDTH/2, SCREEN_HEIGHT + thickness/2, SCREEN_WIDTH, thickness, { isStatic: true }),    // bottom
  Bodies.rectangle(-thickness/2, SCREEN_HEIGHT/2, thickness, SCREEN_HEIGHT, { isStatic: true }),           // left
  Bodies.rectangle(SCREEN_WIDTH + thickness/2, SCREEN_HEIGHT/2, thickness, SCREEN_HEIGHT, { isStatic: true })     // right
];
World.add(engine.world, walls);

// optional: small restitution boost to counter numeric damping (tweak if needed)
Events.on(engine, 'collisionActive', (e) => {
  for (const pair of e.pairs) {
    const { bodyA, bodyB } = pair;
    [bodyA, bodyB].forEach(b => {
      if (!b.isStatic && b.speed > 0) {
        // tiny nudge (1.0003) — adjust or remove if undesirable
        Body.setVelocity(b, { x: b.velocity.x * 1.0003, y: b.velocity.y * 1.0003 });
      }
    });
  }
});

const NUM_QR_CODES_DIR = "../assets/images/number-qrcodes";
const ALPHA_DATA_MATRIX_DIR = "../assets/images/alphabet-data-matrix";
let num_qrcodes = [];
let alpha_dms = [];

let buttons = [];

const margin = IMAGE_DIM/2 + thickness/2;
function getRandomCanvasX() {
  return random(margin, SCREEN_WIDTH-margin);
}

function getRandomCanvasY() {
  return random(margin, SCREEN_HEIGHT-margin);
}

function getRandomColor() {
  return random(["red", "blue", "green", "purple",
                 "pink", "white", "orange", "yellow"]);
}

function getRandomNumQRCodeImage() {
  return `${NUM_QR_CODES_DIR}/${random(["ONE_", "TWO_", "THREE_", "FOUR_", "FIVE_",
                 "SIX_", "SEVEN_", "EIGHT_", "NINE_", "TEN_"])}.png`;
}

function getRandomAlphaDataMatrixImage() {
  return `${ALPHA_DATA_MATRIX_DIR}/${random(["alpha", "beta", "charlie", "delta", "echo",
                 "foxtrot", "golf", "hotel", "india", "juliett"])}.png`;
}

function preload() {
  for (i = 0; i < 10; i++) {
    num_qrcodes[i] = new QRImage(getRandomNumQRCodeImage(), getRandomCanvasX(), getRandomCanvasY(), "yellow");
  }
  for (i = 0; i < 10; i++) {
    alpha_dms[i] = new QRImage(getRandomAlphaDataMatrixImage(), getRandomCanvasX(), getRandomCanvasY(), "red"); 
  }
}

function setup() {
  createCanvas(SCREEN_WIDTH, SCREEN_HEIGHT);

  buttons.push(new Button("Test", testAction));
  buttons.push(new Button("Another One", anotherOneAction));
  buttons.push(new Button("Last One", lastOneAction));
}

function draw() {

  if (SHOW_MENU) {
    background("#660033");
    let x = SCREEN_WIDTH/2;
    let y = SCREEN_HEIGHT/2; 
    for (let b of buttons) {
      b.draw(x, y);
      y += BUTTON_FONT_SIZE + BUTTON_PAD_Y*4;
    }
  } else {

    background("#181818");
   
    Engine.update(engine, 1000/120);


    for (let q of num_qrcodes) {
      q.draw();
    }
    for (let d of alpha_dms) {
      d.draw();
    }
  }
}

function keyPressed() {
  if (keyCode == ESCAPE) SHOW_MENU = !SHOW_MENU;
}

function mouseClicked() {
  for (let b of buttons) {
    if (b.hovered) {
      b.action();
    }
  }
}

function testAction() {
  console.log("Test action!");
}

function anotherOneAction() {
  console.log("Another one action!");
}

function lastOneAction() {
  console.log("Last one action!");
}

class Button {
  constructor(text, action) {
    this.text = text;
    this.hovered = false;
    this.action = action;
  }

  draw(x, y) {
    
    push();
    
    textFont("Courier New");
    textSize(BUTTON_FONT_SIZE);
    
    let tw = textWidth(this.text);
    let w = tw + BUTTON_PAD_X*2;
    let h = BUTTON_FONT_SIZE + BUTTON_PAD_Y*2;
    let tx = x + BUTTON_PAD_X;
    let ty = y + BUTTON_FONT_SIZE + BUTTON_PAD_Y / 2;
    
    this.hovered = mouseX > x &&
                   mouseX < x + w &&
                   mouseY >y &&
                   mouseY < y + h;
    
    noFill();
    stroke(this.hovered ? BUTTON_HOVER_COLOR : BUTTON_COLOR);
    strokeWeight(this.hovered ? 3 : 1);
    rect(x, y, w, h);
   
    fill(this.hovered ? BUTTON_HOVER_COLOR : BUTTON_COLOR);
    text(this.text, tx, ty);
    
    pop();
  }


}

class QRImage {
  constructor(filePath, x, y, clr) {
    this.image = loadImage(filePath);
    this.color = clr;
    this.box = Bodies.rectangle(x, y, IMAGE_DIM, IMAGE_DIM, {
      friction: 0,
      frictionAir: 0,
      frictionStatic: 0,
      restitution: 0.999, 
      inertia: Infinity
    });
    Body.setVelocity(this.box, {
      x: random(-BOX_VELOCITY, BOX_VELOCITY), 
      y: random(-BOX_VELOCITY, BOX_VELOCITY) 
    });
    World.add(engine.world, this.box);
  }

  update() {
  }

  draw() {
    push();
    translate(this.box.position.x, this.box.position.y);
    rotate(this.box.angle);
    imageMode(CENTER);
    //tint(this.color);
    image(this.image, 0, 0, IMAGE_DIM, IMAGE_DIM);
    pop();
  }
}

/*
navigator.mediaDevices.enumerateDevices().then((devices) => {
  devices.forEach((device) => {
    console.log(device); // an InputDeviceInfo object if the device is an input device, otherwise a MediaDeviceInfo object.
  });
});

(async () => {
  let devices = await navigator.usb.getDevices();
  console.log("Devices: ", devices);
})();
*/

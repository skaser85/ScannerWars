/* jshint esversion: 9 */

const SCREEN_WIDTH = 1920;
const SCREEN_HEIGHT = 1080;

const game_state = {
  menu: 0,
  play: 1
}


let game = {
  state: game_state.menu,
  image_dim: 75,
  box_velocity: 10,
}

const DEFAULT_BUTTON_CONFIG = {
  font: "Courier New",
  font_size: 24,
  color: "white",
  hover_color: "aqua",
  pad_x: 20,
  pad_y: 10,
  stroke_weight: 1,
  hover_stroke_weight: 2
};

const DEFAULT_STEPPER_CONFIG = {
  font: "Courier New",
  font_size: 24,
  color: "white",
  pad_x: 20,
  pad_y: 10,
  stroke_weight: 1,
};

const { Engine, World, Bodies, Body, Events } = Matter;

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

function getDefaultButtonConfigClone() {
  return { ...DEFAULT_BUTTON_CONFIG };
}

function getDefaultStepperConfigClone() {
  return { ...DEFAULT_STEPPER_CONFIG };
}

const NUM_QR_CODES_DIR = "../assets/images/number-qrcodes";
const ALPHA_DATA_MATRIX_DIR = "../assets/images/alphabet-data-matrix";
let num_qrcodes = [];
let alpha_dms = [];

let buttons = [];
let steppers = [];

const margin = game.image_dim/2 + thickness/2;
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
                 "SIX_", "SEVEN_", "EIGHT_", "NINE_", "TEN_"])}.png`; }

function getRandomAlphaDataMatrixImage() {
  return `${ALPHA_DATA_MATRIX_DIR}/${random(["alpha", "beta", "charlie", "delta", "echo",
                 "foxtrot", "golf", "hotel", "india", "juliett"])}.png`;
}

function preload() {
  for (let i = 0; i < 10; i++) {
    num_qrcodes[i] = new QRImage(getRandomNumQRCodeImage(), getRandomCanvasX(), getRandomCanvasY(), "yellow");
  }
  for (let i = 0; i < 10; i++) {
    alpha_dms[i] = new QRImage(getRandomAlphaDataMatrixImage(), getRandomCanvasX(), getRandomCanvasY(), "red"); 
  }
}

function setup() {
  createCanvas(SCREEN_WIDTH, SCREEN_HEIGHT);

  steppers.push(new Stepper("Max Box Velocity", maxBoxVelocityStepperIncrButtonAction, maxBoxVelocityStepperDecrButtonAction));
}

function draw() {

  if (game.state === game_state.menu) {
    background("#660033");
    let x = SCREEN_WIDTH/2;
    let y = SCREEN_HEIGHT/2; 
    for (let b of buttons) {
      b.draw(x, y);
      y += b.config.font_size + b.config.pad_y*4;
    }
    for (let s of steppers) {
      s.draw(x, y, game.box_velocity); // TODO: make this not bad
      y += s.config.font_size + s.config.pad_y*4;
    }
  } else if (game.state === game_state.play) {

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
  if (keyCode === ESCAPE) {
    if (game.state === game_state.menu) {
      game.state = game_state.play;
    } else if (game.state === game_state.play) {
      game.state = game_state.menu;
    }
  }
  return false;
}

function mouseClicked() {
  for (let b of buttons) {
    if (b.hovered) {
      b.action();
      return;
    }
  }
  for (let s of steppers) {
    if (s.incr.hovered) {
      s.incr.action();
      return;
    } else if (s.decr.hovered) {
      s.decr.action();
      return;
    }
  }
  
  for (let q of num_qrcodes) {
    if (q.containsPoint(mouseX, mouseY))
      q.tint = !q.tint;
  }
  for (let d of alpha_dms) {
    if (d.containsPoint(mouseX, mouseY))
      d.tint = !d.tint;
  }

}

function maxBoxVelocityStepperIncrButtonAction() {
  game.box_velocity += 1;
  for (let q of num_qrcodes) q.updateVelocity(1, "delta");
  for (let d of alpha_dms)  d.updateVelocity(1, "delta");
}

function maxBoxVelocityStepperDecrButtonAction() {
  game.box_velocity = Math.max(1, game.box_velocity - 1);
  // clamp every body to the new global limit
  for (let q of num_qrcodes) q.updateVelocity(game.box_velocity, "limit");
  for (let d of alpha_dms)  d.updateVelocity(game.box_velocity, "limit");
}

class Button {
  constructor(text, action, config) {
    this.text = text;
    this.hovered = false;
    this.action = action;
    this.config = config ? config : getDefaultButtonConfigClone();
  }
  
  draw(x, y) {
    
    push();
    
    textFont(this.config.font);
    textSize(this.config.font_size);
    
    let tw = textWidth(this.text);
    let w = tw + this.config.pad_x*2;
    let h = this.config.font_size + this.config.pad_y*2;
    let tx = x + this.config.pad_x;
    let ty = y + this.config.font_size + this.config.pad_y / 2;
    
    this.hovered = mouseX > x &&
                   mouseX < x + w &&
                   mouseY >y &&
                   mouseY < y + h;
    
    noFill();
    stroke(this.hovered ? this.config.hover_color: this.config.color);
    strokeWeight(this.hovered ? this.config.hover_stroke_weight : this.config.stroke_weight);
    rect(x, y, w, h);
   
    fill(this.hovered ? this.config.hover_color : this.config.color);
    text(this.text, tx, ty);
    
    pop();
  }

  get_width() {
    
    push();
    
    textFont(this.config.font);
    textSize(this.config.font_size);
    let tw = textWidth(this.text);
    let w = tw + this.config.pad_x*2;

    pop();
    
    return w;
  }

}

class Stepper {
  constructor(text, incr_action, decr_action, button_config, stepper_config) {
    this.text = text;
    this.incr = new Button("+", incr_action, getDefaultButtonConfigClone());
    this.decr = new Button("-", decr_action, getDefaultButtonConfigClone());
    this.config = stepper_config ? stepper_config : getDefaultStepperConfigClone();
  }

  draw(x, y, value) {

    push();
 
    textFont(this.config.font);
    textSize(this.config.font_size);
    
    let tw = textWidth(this.text);
    let w = tw + this.config.pad_x*2 + this.incr.get_width() + this.decr.get_width();
    let h = this.config.font_size + this.config.pad_y*2;
    let tx = x + this.config.pad_x + this.incr.get_width();
    let ty = y + this.config.font_size + this.config.pad_y / 2;
       
    stroke(this.config.color);
    strokeWeight(this.config.stroke_weight);
    fill(this.config.color);
    text(this.text, tx, ty);

    pop();

    this.decr.draw(x, y);
    this.incr.draw(x + w - this.incr.get_width(), y);

    push();
    textFont(this.config.font);
    textSize(this.config.font_size);
    tx = x + w + this.config.pad_x;
    fill(this.config.color);
    text(value, tx, ty);
    pop();
  }
}

class QRImage {
  constructor(filePath, x, y, clr) {
    this.image = loadImage(filePath);
    this.color = clr;
    this.tint = false;
    this.box = Bodies.rectangle(x, y, game.image_dim, game.image_dim, {
      friction: 0,
      frictionAir: 0,
      frictionStatic: 0,
      restitution: 0.999, 
      inertia: Infinity
    });
    Body.setVelocity(this.box, {
      x: random(-game.box_velocity, game.box_velocity), 
      y: random(-game.box_velocity, game.box_velocity) 
    });
    World.add(engine.world, this.box);
  }
  
  updateVelocity(deltaOrNewLimit, mode = "delta") {
    const body = this.box;
    let vx = body.velocity.x, vy = body.velocity.y;
    let speed = Math.hypot(vx, vy);

    const setFromAngleMag = (angle, mag) => Body.setVelocity(body, { x: Math.cos(angle) * mag, y: Math.sin(angle) * mag });

    if (speed === 0) {
      const angle = Math.random() * Math.PI * 2;
      const mag = Math.max(1, Math.min(game.box_velocity, (mode === "limit" ? deltaOrNewLimit : game.box_velocity)));
      setFromAngleMag(angle, mag);
      return;
    }

    const angle = Math.atan2(vy, vx);

    if (mode === "limit") {
      const newLimit = Math.max(1, deltaOrNewLimit);
      const newSpeed = Math.min(speed, newLimit);
      setFromAngleMag(angle, newSpeed);
    } else {
      const newSpeed = Math.max(1, Math.min(game.box_velocity, speed + deltaOrNewLimit));
      setFromAngleMag(angle, newSpeed);
    }
  }

  containsPoint(mx, my) {
    const cx = this.box.position.x, cy = this.box.position.y;
    const angle = this.box.angle;
    const dx = mx - cx, dy = my - cy;
    const rx = dx * Math.cos(-angle) - dy * Math.sin(-angle);
    const ry = dx * Math.sin(-angle) + dy * Math.cos(-angle);
    const half = game.image_dim / 2;
    return rx > -half && rx < half && ry > -half && ry < half;
  }

  draw() {

    push();
    translate(this.box.position.x, this.box.position.y);
    rotate(this.box.angle);
    imageMode(CENTER);
    noTint();
    if (this.tint)
      tint(this.color);
    image(this.image, 0, 0, game.image_dim, game.image_dim);
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

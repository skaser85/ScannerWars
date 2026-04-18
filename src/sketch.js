const SCREEN_WIDTH = 800;
const SCREEN_HEIGHT = 600;
const IMAGE_DIM = 50;
const NUM_QR_CODES_DIR = "../assets/images/number-qrcodes";
let num_qrcodes = [];

function getRandomCanvasX() {
  return random(IMAGE_DIM, SCREEN_WIDTH-IMAGE_DIM);
}

function getRandomCanvasY() {
  return random(IMAGE_DIM, SCREEN_HEIGHT-IMAGE_DIM);
}

function preload() {
  num_qrcodes[0] = new QRImage(`${NUM_QR_CODES_DIR}/ONE_.png`, getRandomCanvasX(), getRandomCanvasY(), 'red');
  num_qrcodes[1] = new QRImage(`${NUM_QR_CODES_DIR}/TWO_.png`, getRandomCanvasX(), getRandomCanvasY(), 'blue');
  num_qrcodes[2] = new QRImage(`${NUM_QR_CODES_DIR}/THREE_.png`, getRandomCanvasX(), getRandomCanvasY(), 'green');
  num_qrcodes[3] = new QRImage(`${NUM_QR_CODES_DIR}/FOUR_.png`, getRandomCanvasX(), getRandomCanvasY(), 'purple');
  num_qrcodes[4] = new QRImage(`${NUM_QR_CODES_DIR}/FIVE_.png`, getRandomCanvasX(), getRandomCanvasY(), 'pink');
  num_qrcodes[5] = new QRImage(`${NUM_QR_CODES_DIR}/SIX_.png`, getRandomCanvasX(), getRandomCanvasY(), 'white');
}

function setup() {
  createCanvas(SCREEN_WIDTH, SCREEN_HEIGHT);

  angleMode(DEGREES);
}

function draw() {
  background("#181818");

  for (let q of num_qrcodes) {
    q.update();
    q.draw();
  }
}

class QRImage {
  constructor(filePath, x, y, clr) {
    this.image = loadImage(filePath);
    this.pos = createVector(x, y);
    this.velocity = createVector(random(-6, 6), random(-6, 6));
    this.colliding = false;
    this.tint = clr;
  }

  update() {

    this.pos.add(this.velocity);
    
    if (this.pos.x >= width-IMAGE_DIM || this.pos.x <= 0) {
      this.velocity.x *= -1;
      this.pos.x += this.velocity.x;
    } else if (this.pos.y >= height-IMAGE_DIM || this.pos.y <= 0) {
      this.velocity.y *= -1;
      this.pos.y += this.velocity.y;
    }

    this.colliding = false;
    for (let q of num_qrcodes) {
      if (q != this) {
        if (this.pos.x < q.pos.x + IMAGE_DIM &&
            this.pos.x + IMAGE_DIM > q.pos.x &&
            this.pos.y < q.pos.y + IMAGE_DIM &&
            this.pos.y + IMAGE_DIM > q.pos.y) {
          this.colliding = true;
          
          this.velocity.mult(-1);
          this.pos.add(this.velocity);

        }
      }
    }
    
  }

  draw() {
    //noTint();
    //if (this.colliding) tint('red');
    tint(this.tint);
    image(this.image, this.pos.x, this.pos.y, IMAGE_DIM, IMAGE_DIM);
  }
}

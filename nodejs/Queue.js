/*

Queue.js

A function to represent a queue

Created by Stephen Morley - http://code.stephenmorley.org/ - and released under
the terms of the CC0 1.0 Universal legal code:

http://creativecommons.org/publicdomain/zero/1.0/legalcode

*/
//1/23/2012
//CLS - Modified Queue function names to match standard Java Queue name conventions

//CLS - modified for node.js import
exports.Queue = Queue;

/* Creates a new queue. A queue is a first-in-first-out (FIFO) data structure -
 * items are added to the end of the queue and removed from the front.
 */
function Queue(){

  // initialise the queue and offset
  var queue  = [];
  var offset = 0;

  /* Returns the length of the queue.
   */
  //CLS - rename from getLength to size
  this.size = function(){

    // return the length of the queue
    return (queue.length - offset);

  }

  /* Returns true if the queue is empty, and false otherwise.
   */
  this.isEmpty = function(){

    // return whether the queue is empty
    return (queue.length == 0);

  }

  /* Enqueues the specified item. The parameter is:
   *
   * item - the item to enqueue
   */
  //CLS - renamed from enqueue to add
  this.add = function(item){

    // enqueue the item
    queue.push(item);

  }

  /* Dequeues an item and returns it. If the queue is empty then undefined is
   * returned.
   */
  //CLS - renamed from dequeue to remove
  this.remove = function(){

    // if the queue is empty, return undefined
    if (queue.length == 0) return undefined;

    // store the item at the front of the queue
    var item = queue[offset];

    // increment the offset and remove the free space if necessary
    if (++ offset * 2 >= queue.length){
      queue  = queue.slice(offset);
      offset = 0;
    }

    // return the dequeued item
    return item;

  }

  /* Returns the item at the front of the queue (without dequeuing it). If the
   * queue is empty then undefined is returned.
   */
  this.peek = function(){

    // return the item at the front of the queue
    return (queue.length > 0 ? queue[offset] : undefined);

  }

  //CLS - Modified to support a contains function (requires '.equals' to work correctly)
  this.contains = function(item) {
    var found = false;
    var selectedSession = null;

    for (var i = 0; i < queue.length; i++)
    {
        if(queue[i].equals(item))
        {
            found = true;
            break;
        }
    }

    return found;
  }
  
  //CLS - add clear function
  this.clear = function() {
    queue = [];
    offset = 0;
  }

}
